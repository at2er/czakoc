/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "conv.h"
#include "czakoc.h"
#include "darr.h"
#include "die.h"
#include "ealloc.h"
#include "macros.h"
#include "parser.h"
#include "zako.h"

#define SCLEXER_IMPL
#include "sclexer.h"

#include "../gen/ast.h"

#define __actually_next(P) sclexer_next(&(P)->lexer, &(P)->tok)
#define __use_peek(P) (((P)->peek = 0) || 1)
#define _next(P) ((P)->peek ? __use_peek(P) : __actually_next(P))

#define duptok(P) strndup((P)->tok.str, (P)->tok.len)
#define expect(P, TOK) if ((P)->tok.type != (TOK)) unexpected(P)
#define next(P) nextor(P) die("%d:%d: EOF\n", (P)->lexer.row, (P)->lexer.col)
#define nextor(P) if (!_next(P))
#define peek(P) ((P)->peek = 1)
#define unexpected(P) \
	die("%d:%d: unexpected token: \"%.*s\"\n", \
	    (P)->tok.row, (P)->tok.col, (P)->tok.len, (P)->tok.str)

struct parser {
	struct sclexer lexer;
	struct sclexer_tok tok;
	unsigned int peek:1;
};

static struct zk_top_stmt *parse(struct parser *p);
static struct zk_ident *parse_dec(struct parser *p);
static int parse_expr(struct parser *p, struct zk_expr *exp);
static struct zk_top_stmt *parse_fn_def(struct parser *p,
		struct zk_top_stmt *top);
static struct zk_fn_type *parse_fn_type(struct parser *p);
static struct zk_stmt *parse_return(struct parser *p);
static struct zk_stmt *parse_stmt(struct parser *p);
static int parse_type(struct parser *p, struct zk_type *typ);
static int parse_value(struct parser *p, struct zk_value *val);

static const char *tokens[] = {
	[TOK_ASSIGN] = "=",
	[TOK_COMMA]  = ",",
	[TOK_LBRACE] = "{",
	[TOK_RBRACE] = "}",
	[TOK_LPAREN] = "(",
	[TOK_RPAREN] = ")",
	[TOK_FN]     = "fn",
	[TOK_RETURN] = "return",
	[TOK_U8]     = "u8",
	[TOK_U16]    = "u16",
	[TOK_U32]    = "u32",
	[TOK_U64]    = "u64",
	[TOK_I8]     = "i8",
	[TOK_I16]    = "i16",
	[TOK_I32]    = "i32",
	[TOK_I64]    = "i64",
	NULL
};
static const char *comments[] = {
	"--",
	NULL
};

struct zk_top_stmt *
parse(struct parser *p)
{
	bool has_assign = false;
	struct zk_ident *id;
	struct zk_top_stmt *stmt;

	id = parse_dec(p);
	if (!id)
		return NULL;

	next(p); /* prepare for definition */
	if (p->tok.type == TOK_ASSIGN)
		has_assign = true;
	else
		peek(p);

	stmt = ecalloc(1, sizeof(*stmt));
	switch (id->typ.base) {
	case ZK_FN:
		stmt->k = TOPSTMT_FN_DEC;
		stmt->u.fn_dec = id;
		if (has_assign)
			return parse_fn_def(p, stmt);
		break;
	default:
		break;
	}

	return stmt;
}

struct zk_ident *
parse_dec(struct parser *p)
{
	struct zk_ident *id = ecalloc(1, sizeof(*id));

	next(p);
	id->name = duptok(p);
	next(p);
	peek(p);
	if (parse_type(p, &id->typ))
		goto err;

	return id;
err:
	free(id->name);
	free(id);
	return NULL;
}

int
parse_expr(struct parser *p, struct zk_expr *exp)
{
	darr_init(&exp->values);
	darr_expand(&exp->values);
	if (parse_value(p, &darr_last(&exp->values)))
		return 1;
	return 0;
}

struct zk_top_stmt *
parse_fn_def(struct parser *p, struct zk_top_stmt *top)
{
	struct zk_ident *id = top->u.fn_dec;
	struct zk_stmt *stmt;
	top->k = TOPSTMT_FN_DEF;
	top->u.fn_def.dec = id;
	next(p);
	expect(p, TOK_LBRACE);

	darr_init(&top->u.fn_def.body);

	while (1) {
		stmt = parse_stmt(p);
		if (!stmt)
			return NULL;
		darr_append(&top->u.fn_def.body, stmt);
	}

	return top;
}

struct zk_fn_type *
parse_fn_type(struct parser *p)
{
	struct zk_ident *arg;
	struct zk_fn_type *fn = ecalloc(1, sizeof(*fn));

	darr_init(&fn->args);

	next(p);
	expect(p, TOK_LPAREN);

	while (1) {
		arg = parse_dec(p);
		darr_append(&fn->args, arg);
		next(p);
		if (p->tok.type == TOK_RPAREN)
			break;
		else if (p->tok.type != TOK_COMMA)
			unexpected(p);
	}

	parse_type(p, &fn->typ);

	return fn;
}

struct zk_stmt *
parse_return(struct parser *p)
{
	struct zk_stmt *stmt = ecalloc(1, sizeof(*stmt));
	stmt->k = STMT_RETURN_STMT;
	if (parse_expr(p, &stmt->u.return_stmt))
		goto err;
	return stmt;
err:
	free(stmt);
	return NULL;
}

struct zk_stmt *
parse_stmt(struct parser *p)
{
	next(p);
	switch (p->tok.type) {
	case TOK_RETURN:
		return parse_return(p);
	default:
		break;
	}
	return NULL;
}

int
parse_type(struct parser *p, struct zk_type *typ)
{
	next(p);
	switch (p->tok.type) {
	case TOK_FN:
		typ->base = ZK_FN;
		typ->u.fn = parse_fn_type(p);
		if (!typ->u.fn)
			return 1;
		return 0;
	default: /* match integer */
		if (BETWEEN(p->tok.type, TOK_U8, TOK_I64))
			typ->base = ZK_8 + p->tok.type - TOK_U8;
		if (BETWEEN(p->tok.type, TOK_I8, TOK_I64)) {
			typ->base -= TOK_I8 - TOK_U8;
			typ->_signed = 1;
		}
		return 0;
	}
	return 1;
}

int
parse_value(struct parser *p, struct zk_value *val)
{
	next(p);
	switch (p->tok.type) {
	case TOK_INT:
		val->u.uint = utilsh_conv_a2ui(p->tok.str, p->tok.len);
		val->k = VALUE_UINT;
		break;
	default:
		return 1;
	}
	return 0;
}

int
parse_file(const char *path)
{
	FILE *fp;
	struct parser p = {0};
	char *src;
	struct zk_top_stmt *stmt;
	struct zk_ast_root ast;

	darr_init(&ast.stmts);

	if (!(fp = fopen(path, "r")))
		return 1;

	sclexer_read_file(fp, &src);
	p.lexer.comments = comments;
	p.lexer.tokens = tokens;
	p.lexer.eol_tok = TOK_EOL;
	p.lexer.ident_tok = TOK_IDENT;
	p.lexer.int_tok = TOK_INT;	
	sclexer_init(&p.lexer, src);
#if 0
	while (1) {
		nextor(&p) break;
		peek(&p);
		if (p.tok.type == TOK_IDENT)
			stmt = parse(&p);
		assert(stmt);
		darr_append(&ast.stmts, stmt);
	}
#else
	while (sclexer_next(&p.lexer, &p.tok)) {
		printf("%d:%d\t%d\t\"%.*s\"\n",
				p.tok.row, p.tok.col, p.tok.type,
				p.tok.len, p.tok.str);
	}
#endif
	return 0;
}
