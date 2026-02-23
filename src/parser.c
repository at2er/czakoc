/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"
#include "czakoc.h"
#include "darr.h"
#include "err.h"
#include "ealloc.h"
#include "ident.h"
#include "jim2.h"
#include "macros.h"
#include "panic.h"
#include "parser.h"
#include "sclexer.h"
#include "semantics.h"
#include "type.h"

#define JIM_PP 2

enum KEYWORD {
	KEYWORD_FN,
	KEYWORD_PUB,
	KEYWORD_RETURN,

	KEYWORD_I8, KEYWORD_I16, KEYWORD_I32, KEYWORD_I64,
	KEYWORD_U8, KEYWORD_U16, KEYWORD_U32, KEYWORD_U64
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
static int get_expr_op_binding_power(enum ZAKO_SYMBOL sym);
static struct zako_expr *merge_expr(
		struct zako_expr *origin,
		struct zako_expr *append);
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
static enum ZAKO_SYMBOL peek_expr_op(struct sclexer_tok *tok);
static struct sclexer_tok *peek_tok(struct parser *parser);
static void print_ast_by_jim(
		struct zako_toplevel_stmt **stmts,
		size_t stmts_count);

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
	[SYM_PAREN_L]   = "(",
	[SYM_PAREN_R]   = ")",
	[SYM_ASSIGN]    = "=",
	[SYM_COMMA]     = ",",
	[SYM_INFIX_ADD] = "+",
	[SYM_INFIX_DIV] = "/",
	[SYM_INFIX_MUL] = "*",
	[SYM_INFIX_SUB] = "-"
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

int
get_expr_op_binding_power(enum ZAKO_SYMBOL sym)
{
	switch (sym) {
	case SYM_INFIX_ADD:
	case SYM_INFIX_SUB:
		return 0;
	case SYM_INFIX_DIV:
	case SYM_INFIX_MUL:
		return 1;
	default:
		break;
	}
	panic("get_expr_op_binding_power()");
	return -1;
}

struct zako_expr *
merge_expr(struct zako_expr *origin, struct zako_expr *append)
{
	struct zako_binary_expr *orig_binary, *append_binary;
	int orig_power, append_power;

	assert(origin && append);
	assert(origin->kind == BINARY_EXPR);
	assert(append->kind == BINARY_EXPR);

	orig_binary   = &origin->inner.binary;
	orig_power    = get_expr_op_binding_power(orig_binary->op);
	append_binary = &append->inner.binary;
	append_power  = get_expr_op_binding_power(append_binary->op);

	if (orig_power >= append_power) {
		append_binary->lhs = ecalloc(1, sizeof(*append_binary->lhs));
		append_binary->lhs->kind = EXPR_LITERAL;
		append_binary->lhs->data.expr = origin;
		return append;
	} else {
		append_binary->lhs = orig_binary->rhs;
		orig_binary->rhs = ecalloc(1, sizeof(*orig_binary->rhs));
		orig_binary->rhs->kind = EXPR_LITERAL;
		orig_binary->rhs->data.expr = append;
		return origin;
	}
	return origin;
}

struct zako_expr *
parse_expr(struct zako_fn_definition *fn, struct parser *parser)
{
	struct zako_expr *expr, *append_expr;
	struct zako_literal *literal;
	enum ZAKO_SYMBOL op = 0;
	struct sclexer_tok *tok;
	assert(fn && parser);
	literal = parse_literal(fn, parser);
	if (!literal)
		return NULL;
	expr = ecalloc(1, sizeof(*expr));
	expr->kind = PRIMARY_EXPR;
	expr->inner.primary = literal;
	tok = peek_tok(parser);
	while ((op = peek_expr_op(tok)) != 0) {
		eat_tok(parser);
		append_expr = ecalloc(1, sizeof(*append_expr));
		append_expr->kind = BINARY_EXPR;
		append_expr->inner.binary.op = op;
		append_expr->inner.binary.rhs = parse_literal(fn, parser);
		if (expr->kind == PRIMARY_EXPR) {
			append_expr->inner.binary.lhs = literal;
			free(expr);
			expr = append_expr;
		} else {
			expr = merge_expr(expr, append_expr);
		}
		tok = peek_tok(parser);
	}

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
	if (tok->kind != SCLEXER_SYMBOL)
		goto err_unexpected_token;
	if (tok->data.symbol != SYM_PAREN_R)
		goto err_unexpected_symbol;
	eat_tok(parser);
	tok = peek_tok(parser);
	type->type = parse_type(parser);
	if (!type->type)
		goto err_free_args;
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
	if (!self->expr)
		goto err_free_stmt;
	if (analyse_expr(self->expr, fn->declaration->ident
				->type->inner.fn.type))
		goto err_free_stmt;
	return stmt;
err_free_stmt:
	free_zako_stmt(stmt);
	return NULL;
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
	switch (tok->data.keyword) {
	case KEYWORD_RETURN:
		return parse_return_stmt(fn, parser);
		break;
	default:
		break;
	}
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

enum ZAKO_SYMBOL
peek_expr_op(struct sclexer_tok *tok)
{
	if (tok->kind != SCLEXER_SYMBOL)
		return 0;
	if (tok->data.symbol < SYM_INFIX_ADD)
		return 0;
	if (tok->data.symbol > SYM_INFIX_SUB)
		return 0;
	return tok->data.symbol;
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
print_ast_by_jim(
		struct zako_toplevel_stmt **stmts,
		size_t stmts_count)
{
	Jim jim = {.pp = JIM_PP};
	jim_array_begin(&jim);
	for (size_t i = 0; i < stmts_count; i++)
		print_toplevel_stmt(stmts[i], &jim);
	jim_array_end(&jim);
	fwrite(jim.sink, jim.sink_count, 1, stdout);
	fwrite("\n", sizeof(char), 1, stdout);
}

void
free_zako_expr(struct zako_expr *self)
{
	if (!self)
		return;
	switch (self->kind) {
	case BINARY_EXPR:
		free_zako_literal(self->inner.binary.lhs);
		free_zako_literal(self->inner.binary.rhs);
		break;
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
	if (czakoc_flags & CZAKOC_OUTPUT_LEXER_TOKENS) {
		for (size_t i = 0; i < parser.tokens_count; i++)
			sclexer_print_tok(&lexer, &parser.tokens[i]);
	}

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
	if (czakoc_flags & CZAKOC_OUTPUT_AST)
		print_ast_by_jim(stmts, stmts_count);
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

void
print_expr(struct zako_expr *self, Jim *jim)
{
	Jim fallback = {.pp = JIM_PP};
	if (!jim)
		jim = &fallback;
	jim_object_begin(jim);
	jim_member_key(jim, "kind");
	switch (self->kind) {
	case BINARY_EXPR:
		jim_string(jim, "binary");
		jim_member_key(jim, "op");
		jim_string(jim, symbols[self->inner.binary.op]);
		jim_member_key(jim, "lhs");
		print_literal(self->inner.binary.lhs, jim);
		jim_member_key(jim, "rhs");
		print_literal(self->inner.binary.rhs, jim);
		break;
	case PRIMARY_EXPR:
		jim_string(jim, "primary");
		jim_member_key(jim, "literal");
		print_literal(self->inner.primary, jim);
		break;
	}
	jim_member_key(jim, "type");
	print_type(self->type, jim);
	jim_object_end(jim);
}

void
print_fn_definition(struct zako_fn_definition *self, Jim *jim)
{
	Jim fallback = {.pp = JIM_PP};
	struct zako_fn_type *fn_type;
	if (!jim)
		jim = &fallback;
	jim_object_begin(jim);
	jim_member_key(jim, "kind");
	jim_string(jim, "function definition");
	fn_type = &self->declaration->ident->type->inner.fn;
	jim_member_key(jim, "type");
	print_type(fn_type->type, jim);
	jim_member_key(jim, "args");
	jim_array_begin(jim);
	for (int i = 0; i < fn_type->argc; i++)
		print_ident(fn_type->args[i], jim);
	jim_array_end(jim);
	jim_member_key(jim, "statements");
	jim_array_begin(jim);
	for (size_t i = 0; i < self->stmts_count; i++)
		print_stmt(self->stmts[i], jim);
	jim_array_end(jim);
	jim_object_end(jim);
}

void
print_ident(struct zako_ident *self, Jim *jim)
{
	Jim fallback = {.pp = JIM_PP};
	if (!jim)
		jim = &fallback;
	jim_object_begin(jim);
	jim_member_key(jim, "kind");
	jim_string(jim, "identifier");
	jim_member_key(jim, "name");
	jim_string(jim, self->name);
	jim_member_key(jim, "type");
	print_type(self->type, jim);
	jim_object_end(jim);
}

void
print_literal(struct zako_literal *self, Jim *jim)
{
	Jim fallback = {.pp = JIM_PP};
	if (!jim)
		jim = &fallback;
	jim_object_begin(jim);
	jim_member_key(jim, "kind");
	switch (self->kind) {
	case EXPR_LITERAL:
		jim_string(jim, "expr literal");
		print_expr(self->data.expr, jim);
		break;
	case INT_LITERAL:
		jim_string(jim, "int literal");
		jim_member_key(jim, "int");
		jim_integer(jim, self->data.i);
		break;
	}
	jim_member_key(jim, "type");
	print_type(self->type, jim);
	jim_object_end(jim);
}

void
print_stmt(struct zako_stmt *self, Jim *jim)
{
	Jim fallback = {.pp = JIM_PP};
	if (!jim)
		jim = &fallback;
	jim_object_begin(jim);
	jim_member_key(jim, "kind");
	switch (self->kind) {
	case RETURN_STMT:
		jim_string(jim, "return");
		jim_member_key(jim, "expr");
		print_expr(self->inner.return_stmt->expr, jim);
		break;
	}
	jim_object_end(jim);
}

void
print_toplevel_stmt(struct zako_toplevel_stmt *self, Jim *jim)
{
	Jim fallback = {.pp = JIM_PP};
	if (!jim)
		jim = &fallback;
	switch (self->kind) {
	case DECLARATION_STMT:
		return;
	case DEFINITION_STMT:
		switch (self->inner.definition.kind) {
		case FN_DEFINITION:
			print_fn_definition(
					self->inner.definition.inner
					.fn_defintion,
					jim);
			break;
		}
		break;
	}
}

void
print_type(struct zako_type *self, Jim *jim)
{
	Jim fallback = {.pp = JIM_PP};
	if (!jim)
		jim = &fallback;
	jim_object_begin(jim);
	jim_member_key(jim, "kind");
	jim_string(jim, "type");
	jim_member_key(jim, "builtin");
	if (self->builtin <= U64_TYPE) {
		jim_string(jim, keywords[KEYWORD_I8 + self->builtin]);
	} else if (self->builtin == FN_TYPE) {
		jim_string(jim, "function");
	}
	jim_object_end(jim);
}
