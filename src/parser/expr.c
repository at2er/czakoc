/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "expr.h"
#include "parser.h"
#include "sclexer.h"
#include "scope.h"
#include "utils.h"
#include "value.h"
#include "../ealloc.h"
#include "../err.h"
#include "../expr.h"
#include "../lexer.h"
#include "../panic.h"
#include "../semantics/expr.h"
#include "../semantics/semantics.h"

enum EXPR_PARSING_MODE {
	CMP_EXPR_MODE,
	DEFAULT_EXPR_MODE
};

static int get_expr_op_binding_power(enum ZAKO_SYMBOL sym);
static bool is_expr_stmt_op(struct sclexer_tok *tok);
static struct zako_expr *merge_expr(
		struct zako_expr *origin,
		struct zako_expr *append);
static struct zako_expr *parse_expr_with_mode(
		struct parser *parser,
		bool in_paren,
		enum EXPR_PARSING_MODE mode);
static enum ZAKO_SYMBOL peek_expr_op(
		struct sclexer_tok *tok,
		struct parser *parser,
		bool in_paren,
		enum EXPR_PARSING_MODE mode);

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

bool
is_expr_stmt_op(struct sclexer_tok *tok)
{
	assert(tok);
	if (tok->kind != SCLEXER_SYMBOL)
		return false;
	switch (tok->data.symbol) {
	case SYM_ASSIGN:
		return true;
	default:
		break;
	}
	return false;
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
		append_binary->lhs->kind = EXPR_VALUE;
		append_binary->lhs->data.expr = origin;
		return append;
	}

	append_binary->lhs = orig_binary->rhs;
	orig_binary->rhs = ecalloc(1, sizeof(*orig_binary->rhs));
	orig_binary->rhs->kind = EXPR_VALUE;
	orig_binary->rhs->data.expr = append;
	return origin;
}

struct zako_expr *
parse_expr_with_mode(
		struct parser *parser,
		bool in_paren,
		enum EXPR_PARSING_MODE mode)
{
	struct zako_expr *expr, *append_expr;
	enum ZAKO_SYMBOL op = 0;
	struct sclexer_tok *tok;
	struct zako_value *value;
	assert(parser);
	value = parse_value(parser);
	if (!value)
		return NULL;
	expr = ecalloc(1, sizeof(*expr));
	expr->kind = PRIMARY_EXPR;
	expr->inner.primary = value;
	tok = peek_tok(parser);
	while ((op = peek_expr_op(tok, parser, in_paren, mode)) != 0) {
		eat_tok(parser);
		append_expr = ecalloc(1, sizeof(*append_expr));
		if (mode == DEFAULT_EXPR_MODE)
			append_expr->kind = BINARY_EXPR;
		else
			append_expr->kind = CMP_EXPR;
		append_expr->inner.binary.op = op;
		append_expr->inner.binary.rhs = parse_value(parser);
		if (expr->kind == PRIMARY_EXPR) {
			append_expr->inner.binary.lhs = value;
			free(expr);
			expr = append_expr;
		} else {
			/* in CMP_EXPR_MODE, this branch never be ran */
			expr = merge_expr(expr, append_expr);
		}
		if (expr->kind == CMP_EXPR)
			break;
		tok = peek_tok(parser);
	}

	return expr;
}

enum ZAKO_SYMBOL
peek_expr_op(
		struct sclexer_tok *tok,
		struct parser *parser,
		bool in_paren,
		enum EXPR_PARSING_MODE mode)
{
	if (tok->kind == SCLEXER_EOL && in_paren) {
		eat_tok(parser);
		tok = peek_tok(parser);
	}
	if (tok->kind != SCLEXER_SYMBOL)
		return 0;
	if (mode == CMP_EXPR_MODE) {
		if (tok->data.symbol < SYM_INFIX_LESS_EQUAL)
			return 0;
		if (tok->data.symbol > SYM_INFIX_LESS_EQUAL)
			return 0;
		return tok->data.symbol;
	}
	if (tok->data.symbol < SYM_INFIX_ADD)
		return 0;
	if (tok->data.symbol > SYM_INFIX_SUB)
		return 0;
	return tok->data.symbol;
}

struct zako_expr *
parse_cmp_expr(struct parser *parser, bool in_paren)
{
	struct zako_expr *expr =
		parse_expr_with_mode(parser, in_paren, CMP_EXPR_MODE);
	if (!expr)
		return NULL;
	expr->type = ecalloc(1, sizeof(*expr->type));
	expr->type->builtin = CMP_EXPR_TYPE;
	return expr;
}

struct zako_expr *
parse_expr(struct parser *parser, bool in_paren)
{
	return parse_expr_with_mode(parser, in_paren, DEFAULT_EXPR_MODE);
}

struct zako_stmt *
parse_expr_stmt(struct sclexer_tok *tok, struct parser *parser)
{
	struct sclexer_tok *begin;
	struct zako_ident *ident;
	char *ident_name;
	int ret;
	struct zako_expr *self;
	struct zako_stmt *stmt;
	assert(tok && parser);
	if (tok->kind != SCLEXER_IDENT)
		goto err_unexpected_token;
	begin = tok;
	ident_name = dup_slice_to_cstr(&tok->data.str);
	ident = find_ident_in_scope(ident_name, parser->cur_scope);
	if (!ident)
		goto err_ident_not_found;
	free(ident_name);
	if (ident->type->builtin == FN_TYPE)
		panic("unsupport syntax");
	tok = eat_tok(parser);
	if (!is_expr_stmt_op(tok))
		goto err_not_expr_stmt;
	stmt = ecalloc(1, sizeof(*stmt));
	stmt->kind = EXPR_STMT;
	stmt->inner.expr_stmt = self = ecalloc(1, sizeof(*self));
	self->kind = BINARY_EXPR;
	self->inner.binary.lhs = ecalloc(1, sizeof(*self->inner.binary.lhs));
	self->inner.binary.lhs->kind = IDENT_VALUE;
	self->inner.binary.lhs->data.ident = ident;
	self->inner.binary.op = tok->data.symbol;
	self->inner.binary.rhs = ecalloc(1, sizeof(*self->inner.binary.rhs));
	self->inner.binary.rhs->kind = EXPR_VALUE;
	self->inner.binary.rhs->data.expr = parse_expr(parser, false);
	ret = analyse_expr_stmt(self);
	if (ret)
		goto err_analyse_expr_stmt;
	return stmt;
err_unexpected_token:
	print_err("unexpected token", tok);
	return NULL;
err_ident_not_found:
	printf_err("identifier '%s' not found", tok, ident_name);
	free(ident_name);
	return NULL;
err_not_expr_stmt:
	print_err("not expression statement", begin);
	return NULL;
err_analyse_expr_stmt:
	printf_err("expr stmt: %s", begin, cstr_analysis_result(ret));
	free_stmt(stmt);
	return NULL;
}
