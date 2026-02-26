/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "expr.h"
#include "parser.h"
#include "sclexer.h"
#include "utils.h"
#include "value.h"
#include "../ealloc.h"
#include "../expr.h"
#include "../lexer.h"
#include "../panic.h"

enum EXPR_PARSING_MODE {
	CMP_EXPR_MODE,
	DEFAULT_EXPR_MODE
};

static int get_expr_op_binding_power(enum ZAKO_SYMBOL sym);
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
