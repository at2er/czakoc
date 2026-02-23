/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"
#include "darr.h"
#include "err.h"
#include "ealloc.h"
#include "ident.h"
#include "macros.h"
#include "parser.h"
#include "sclexer.h"
#include "type.h"

enum KEYWORD {
	KEYWORD_FN,
	KEYWORD_PUB,
	KEYWORD_RETURN,

	KEYWORD_I8, KEYWORD_I16, KEYWORD_I32, KEYWORD_I64,
	KEYWORD_U8, KEYWORD_U16, KEYWORD_U32, KEYWORD_U64
};

enum SYMBOL {
	SYM_PAREN_L,
	SYM_PAREN_R,
	SYM_ASSIGN,
	SYM_COMMA
};

struct parser {
	size_t cur_tok;
	struct sclexer_tok *tokens;
	size_t tokens_count;

	struct zako_module *mod;
};

static void destory_zako_declaration(struct zako_declaration *self);
static void destory_zako_definition(struct zako_definition *self);
static char *dup_slice_to_cstr(struct sclexer_str_slice *slice);
static struct sclexer_tok *eat_tok(struct parser *parser);
static struct zako_expr *parse_expr(
		struct zako_fn_definition *fn,
		struct parser *parser);
static struct zako_fn_definition *parse_fn_body(
		struct zako_fn_declaration *declaration,
		struct parser *parser);
static struct zako_fn_declaration *parse_fn_declaration(
		struct sclexer_tok *tok,
		struct parser *parser);
static struct zako_toplevel_stmt *parse_fn_definition(
		struct sclexer_tok *tok,
		struct parser *parser);
static int parse_fn_sign(struct zako_fn_type *type, struct parser *parser);
static struct zako_ident *parse_ident_sign(
		struct sclexer_tok *tok,
		struct parser *parser);
static struct zako_literal *parse_literal(
		struct zako_fn_definition *fn,
		struct parser *parser);
static struct zako_stmt *parse_return_stmt(
		struct zako_fn_definition *fn,
		struct parser *parser);
static struct zako_stmt *parse_stmt(
		struct sclexer_tok *tok,
		struct zako_fn_definition *fn,
		struct parser *parser);
static struct zako_toplevel_stmt *parse_toplevel_stmt(
		struct sclexer_tok *tok,
		struct parser *parser);
static struct zako_type *parse_type(struct parser *parser);
static struct sclexer_tok *peek_tok(struct parser *parser);

static const char *comments[1] = {";"};
static const char *keywords[] = {
	[KEYWORD_FN]     = "fn",
	[KEYWORD_PUB]    = "pub",
	[KEYWORD_RETURN] = "return",

	[KEYWORD_I8]  = "i8",
	[KEYWORD_I16] = "i16",
	[KEYWORD_I32] = "i32",
	[KEYWORD_I64] = "i64",
	[KEYWORD_U8]  = "u8",
	[KEYWORD_U16] = "u16",
	[KEYWORD_U32] = "u32",
	[KEYWORD_U64] = "u64"
};
static const char *symbols[] = {
	[SYM_PAREN_L] = "(",
	[SYM_PAREN_R] = ")",
	[SYM_ASSIGN]  = "=",
	[SYM_COMMA]   = ","
};

void
destory_zako_declaration(struct zako_declaration *self)
{
	if (!self)
		return;
	switch (self->kind) {
	case FN_DECLARATION:
		free_zako_fn_declaration(self->inner.fn_declaration);
		break;
	}
}

void
destory_zako_definition(struct zako_definition *self)
{
	if (!self)
		return;
	switch (self->kind) {
	case FN_DEFINITION:
		free_zako_fn_definition(self->inner.fn_defintion);
		break;
	}
}

char *
dup_slice_to_cstr(struct sclexer_str_slice *slice)
{
	char *res;
	res = calloc(slice->len + 1, sizeof(char));
	res[slice->len] = '\0';
	strncpy(res, slice->begin, slice->len);
	return res;
}

struct sclexer_tok *
eat_tok(struct parser *parser)
{
	assert(parser);
	if (parser->cur_tok >= parser->tokens_count)
		return NULL;
	parser->cur_tok++;
	assert(parser->tokens);
	if (parser->tokens[parser->cur_tok - 1].kind == SCLEXER_EOL)
		return eat_tok(parser);
	return &parser->tokens[parser->cur_tok - 1];
}

struct zako_expr *
parse_expr(struct zako_fn_definition *fn, struct parser *parser)
{
	struct zako_expr *expr;
	struct zako_literal *literal;
	assert(fn && parser);
	literal = parse_literal(fn, parser);
	if (!literal)
		return NULL;
	expr = ecalloc(1, sizeof(*expr));
	expr->kind = PRIMARY_EXPR;
	expr->inner.primary = literal;
	return expr;
}

struct zako_fn_definition *
parse_fn_body(struct zako_fn_declaration *declaration, struct parser *parser)
{
	struct zako_fn_definition *definition;
	struct zako_stmt *stmt;
	struct sclexer_tok *tok;
	assert(parser);

	definition = ecalloc(1, sizeof(*definition));
	definition->declaration = declaration;

	eat_tok(parser);
	tok = eat_tok(parser);
	if (tok->kind == SCLEXER_EOL) {
		tok = eat_tok(parser);
	} else if (tok->kind != SCLEXER_INDENT_BLOCK_BEGIN) {
		stmt = parse_stmt(tok, definition, parser);
		if (!stmt)
			goto err_free_definition;
		darr_append(definition->stmts, definition->stmts_count, stmt);
		tok = eat_tok(parser);
	}

	if (tok->kind != SCLEXER_INDENT_BLOCK_BEGIN)
		goto err_unexpected_token;
	tok = eat_tok(parser);
	while (tok->kind != SCLEXER_INDENT_BLOCK_END) {
		stmt = parse_stmt(tok, definition, parser);
		if (!stmt)
			goto err_free_definition;
		darr_append(definition->stmts, definition->stmts_count, stmt);
		tok = eat_tok(parser);
	}
	return definition;
err_unexpected_token:
	print_err("unexpected token", tok);
err_free_definition:
	free_zako_fn_definition(definition);
	return NULL;
}

struct zako_fn_declaration *
parse_fn_declaration(struct sclexer_tok *tok, struct parser *parser)
{
	struct zako_fn_declaration *declaration;
	struct zako_ident *ident;
	assert(tok && parser);
	assert(tok->kind == SCLEXER_IDENT);
	ident = parse_ident_sign(tok, parser);
	if (!ident)
		return NULL;
	declaration = ecalloc(1, sizeof(*declaration));
	declaration->ident = ident;
	return declaration;
}

struct zako_toplevel_stmt *
parse_fn_definition(struct sclexer_tok *tok, struct parser *parser)
{
	struct zako_fn_declaration *declaration;
	struct zako_toplevel_stmt *stmt;
	assert(tok && parser);
	declaration = parse_fn_declaration(tok, parser);
	if (!declaration)
		return NULL;
	stmt = ecalloc(1, sizeof(*stmt));
	stmt->kind = DECLARATION_STMT;
	stmt->inner.declaration.kind = FN_DECLARATION;
	stmt->inner.declaration.inner.fn_declaration = declaration;
	tok = peek_tok(parser);
	if (tok->kind != SCLEXER_SYMBOL || tok->data.symbol != SYM_ASSIGN) {
		if (tok->kind != SCLEXER_EOL || tok->kind != SCLEXER_EOF)
			goto err_unexpected_token;
		return stmt; /* declaration */
	}
	stmt->inner.definition.inner.fn_defintion =
		parse_fn_body(declaration, parser);
	if (!stmt->inner.definition.inner.fn_defintion)
		goto err_parse_fn_body;
	stmt->kind = DEFINITION_STMT;
	stmt->inner.definition.kind = FN_DEFINITION;
	return stmt;
err_unexpected_token:
	print_err("unexpected token", tok);
err_parse_fn_body:
	stmt->inner.declaration.inner.fn_declaration = declaration;
	free_zako_toplevel_stmt(stmt);
	return NULL;
}

int
parse_fn_sign(struct zako_fn_type *type, struct parser *parser)
{
	struct zako_ident *arg;
	struct sclexer_tok *tok;
	assert(type && parser);
	tok = peek_tok(parser);
	if (tok->kind == SCLEXER_SYMBOL) {
		if (tok->data.symbol != SYM_PAREN_L)
			goto err_unexpected_symbol;
		eat_tok(parser);
	} else if (tok->kind == SCLEXER_KEYWORD) {
		type->type = parse_type(parser);
		return 0;
	} else {
		goto err_unexpected_token;
	}
	tok = eat_tok(parser);
	arg = parse_ident_sign(tok, parser);
	if (!arg)
		return 1;
	darr_append(type->args, type->argc, arg);
	tok = peek_tok(parser);
	while (tok->kind == SCLEXER_SYMBOL && tok->data.symbol == SYM_COMMA) {
		eat_tok(parser);
		tok = eat_tok(parser);
		arg = parse_ident_sign(tok, parser);
		if (!arg)
			goto err_free_args;
		darr_append(type->args, type->argc, arg);
		tok = peek_tok(parser);
	}
	return 0;
err_unexpected_symbol:
	printf_err("unexpected symbol '%s'", tok, symbols[tok->data.symbol]);
	return 1;
err_unexpected_token:
	print_err("unexpected token", tok);
	return 1;
err_free_args:
	destory_zako_fn_type(type);
	return 1;
}

struct zako_ident *
parse_ident_sign(struct sclexer_tok *tok, struct parser *parser)
{
	struct zako_ident *ident;
	assert(tok && parser);
	assert(tok->kind == SCLEXER_IDENT);
	ident = ecalloc(1, sizeof(*ident));
	ident->name = dup_slice_to_cstr(&tok->data.str);
	ident->type = parse_type(parser);
	if (!ident->type)
		goto err_parse_type;
	return ident;
err_parse_type:
	free(ident->name);
	free(ident);
	return NULL;
}

struct zako_literal *
parse_literal(struct zako_fn_definition *fn, struct parser *parser)
{
	struct zako_literal *literal;
	struct sclexer_tok *tok;
	assert(fn && parser);
	tok = eat_tok(parser);
	if (tok->kind != SCLEXER_INT && tok->kind != SCLEXER_INT_NEG)
		return NULL;
	literal = ecalloc(1, sizeof(literal));
	literal->kind = INT_LITERAL;
	literal->data.i = tok->data.sint;
	return literal;
}

struct zako_stmt *
parse_return_stmt(struct zako_fn_definition *fn, struct parser *parser)
{
	struct zako_return_stmt *self;
	struct zako_stmt *stmt;
	assert(fn && parser);
	stmt = ecalloc(1, sizeof(*stmt));
	stmt->kind = RETURN_STMT;
	stmt->inner.return_stmt = self = ecalloc(1, sizeof(*self));
	self->expr = parse_expr(fn, parser);
	return stmt;
}

struct zako_stmt *
parse_stmt(
		struct sclexer_tok *tok,
		struct zako_fn_definition *fn,
		struct parser *parser)
{
	assert(tok && fn && parser);
	if (tok->kind != SCLEXER_KEYWORD)
		return NULL; /* expression statement */
	switch ((enum KEYWORD)tok->data.keyword) {
	case KEYWORD_RETURN:
		return parse_return_stmt(fn, parser);
		break;
	default:
		goto err_unexpected_keyword;
		break;
	}
	assert(0);
	return NULL;
err_unexpected_keyword:
	printf_err("unexpected keyword '%s'", tok, keywords[tok->data.keyword]);
	return NULL;
}

struct zako_toplevel_stmt *
parse_toplevel_stmt(struct sclexer_tok *tok, struct parser *parser)
{
	assert(tok && parser);
	/* Definition also handle declaration,
	 * but when declaration is correct and not have definition,
	 * fallback to declaration. */
	if (tok->kind == SCLEXER_IDENT)
		return parse_fn_definition(tok, parser);
	return NULL;
}

struct zako_type *
parse_type(struct parser *parser)
{
	struct sclexer_tok *tok;
	struct zako_type *type;
	assert(parser);
	tok = eat_tok(parser);
	if (tok->kind != SCLEXER_KEYWORD)
		goto err_unexpected_token;
	type = ecalloc(1, sizeof(*type));
	switch (tok->data.keyword) {
	case KEYWORD_FN:
		type->builtin = FN_TYPE;
		if (parse_fn_sign(&type->inner.fn, parser))
			goto err_free_type;
		break;
	case KEYWORD_I8:  type->builtin = I8_TYPE;  break;
	case KEYWORD_I16: type->builtin = I16_TYPE; break;
	case KEYWORD_I32: type->builtin = I32_TYPE; break;
	case KEYWORD_I64: type->builtin = I64_TYPE; break;
	case KEYWORD_U8:  type->builtin = U8_TYPE;  break;
	case KEYWORD_U16: type->builtin = U16_TYPE; break;
	case KEYWORD_U32: type->builtin = U32_TYPE; break;
	case KEYWORD_U64: type->builtin = U64_TYPE; break;
	default: goto err_unexpected_keyword; break;
	}
	return type;
err_unexpected_token:
	print_err("unexpected token", tok);
	return NULL;
err_unexpected_keyword:
	printf_err("unexpected keyword '%s'", tok, keywords[tok->data.keyword]);
err_free_type:
	free(type);
	return NULL;
}

struct sclexer_tok *
peek_tok(struct parser *parser)
{
	assert(parser);
	if (parser->cur_tok >= parser->tokens_count)
		return NULL;
	assert(parser->tokens);
	return &parser->tokens[parser->cur_tok];
}

void
free_zako_expr(struct zako_expr *self)
{
	if (!self)
		return;
	switch (self->kind) {
	case PRIMARY_EXPR:
		free_zako_literal(self->inner.primary);
		break;
	}
	free(self);
}

void
free_zako_fn_declaration(struct zako_fn_declaration *self)
{
	if (!self)
		return;
	free_zako_ident(self->ident);
	free(self);
}

void
free_zako_fn_definition(struct zako_fn_definition *self)
{
	if (!self)
		return;
	for (size_t i = 0; i < self->stmts_count; i++)
		free_zako_stmt(self->stmts[i]);
	free(self->stmts);
	free(self);
}

void
free_zako_literal(struct zako_literal *self)
{
	if (!self)
		return;
	free(self);
}

void
free_zako_return_stmt(struct zako_return_stmt *self)
{
	if (!self)
		return;
	free_zako_expr(self->expr);
	free(self);
}

void
free_zako_stmt(struct zako_stmt *self)
{
	if (!self)
		return;
	switch (self->kind) {
	case RETURN_STMT:
		free_zako_return_stmt(self->inner.return_stmt);
		break;
	}
	free(self);
}

void
free_zako_toplevel_stmt(struct zako_toplevel_stmt *self)
{
	if (!self)
		return;
	switch (self->kind) {
	case DECLARATION_STMT:
		destory_zako_declaration(&self->inner.declaration);
		break;
	case DEFINITION_STMT:
		destory_zako_definition(&self->inner.definition);
		break;
	}
	free(self);
}

struct zako_module *
parse_file(const char *path)
{
	struct sclexer_tok *cur;
	struct parser parser = {0};
	struct sclexer lexer = {0};
	struct zako_module *mod;
	char *src;
	struct zako_toplevel_stmt *stmt;
	struct zako_toplevel_stmt **stmts = NULL;
	size_t stmts_count = 0;
	assert(path);

	mod = ecalloc(1, sizeof(*mod));
	strcpy(mod->file_path, path);

	lexer.enable_indent = true;
	lexer.comments = comments;
	lexer.comments_count = LENGTH(comments);
	lexer.keywords = keywords;
	lexer.keywords_count = LENGTH(keywords);
	lexer.symbols = symbols;
	lexer.symbols_count = LENGTH(symbols);
	lexer.src_siz = sclexer_read_file(&src, path);
	lexer.src = src;
	sclexer_init(&lexer, path);

	parser.tokens_count = sclexer_get_tokens(&lexer, &parser.tokens);
	for (size_t i = 0; i < parser.tokens_count; i++)
		sclexer_print_tok(&lexer, &parser.tokens[i]);

	while (parser.cur_tok < parser.tokens_count) {
		cur = eat_tok(&parser);
		if (!cur)
			break;
		switch (cur->kind) {
		case SCLEXER_EOF:
			goto end;
		case SCLEXER_EOL:
			continue;
		case SCLEXER_IDENT:
		case SCLEXER_KEYWORD:
			stmt = parse_toplevel_stmt(cur, &parser);
			if (!stmt)
				goto err_free_all;
			darr_append(stmts, stmts_count, stmt);
			break;
		default:
			goto err_unknown_token;
		}
	}
end:
	if (compile_file(stmts, stmts_count, mod))
		goto err_compile_file;
	free(src);
	return mod;
err_unknown_token:
	print_err("unkown token", cur);
	goto err_free_all;
err_compile_file:
	printf_err_msg("compile file '%s'", path);
err_free_all:
	free(src);
	free(mod);
	return NULL;
}
