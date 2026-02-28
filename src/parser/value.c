/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdbool.h>
#include "expr.h"
#include "parser.h"
#include "scope.h"
#include "utils.h"
#include "value.h"
#include "../darr.h"
#include "../ealloc.h"
#include "../err.h"
#include "../lexer.h"
#include "../value.h"

static int parse_arr_elem_value(
		struct zako_value *value,
		struct zako_ident *arr,
		struct parser *parser);
static int parse_elem_init_value(struct zako_value *value, struct parser *parser);
static int parse_expr_value(struct zako_value *value, struct parser *parser);
static int parse_value_by_sclexer_symbol(
		struct sclexer_tok *tok,
		struct zako_value *value,
		struct parser *parser);
static int parse_value_by_sclexer_ident(
		struct sclexer_tok *tok,
		struct zako_value *value,
		struct parser *parser);

int
parse_arr_elem_value(
		struct zako_value *value,
		struct zako_ident *arr,
		struct parser *parser)
{
	struct sclexer_tok *tok;
	assert(value && arr && parser);

	tok = eat_tok(parser);
	if (tok->kind != SCLEXER_SYMBOL || tok->data.symbol != SYM_BRACKET_L)
		goto err_unexpected_token;

	tok = eat_tok(parser);
	if (tok->kind != SCLEXER_INT)
		goto err_unexpected_token;
	value->kind = ARR_ELEM_VALUE;
	value->data.arr_elem.arr = arr;
	value->data.arr_elem.idx = tok->data.uint;

	tok = eat_tok(parser);
	if (tok->kind != SCLEXER_SYMBOL || tok->data.symbol != SYM_BRACKET_R)
		goto err_unexpected_token;

	return 0;
err_unexpected_token:
	print_err("unexpected token", tok);
	return 1;
}

int
parse_elem_init_value(struct zako_value *value, struct parser *parser)
{
	struct zako_elem_init_value *elem_init;
	struct sclexer_tok *tok;
	struct zako_value *elem;
	assert(value && parser);
	value->kind = ELEM_INIT_VALUE;
	elem_init = &value->data.elem_init;
	tok = peek_tok(parser);
	while (tok->kind != SCLEXER_SYMBOL || tok->data.symbol != SYM_BRACE_R) {
		elem = parse_value(parser);
		if (!elem)
			goto err_elem_init_not_end;
		darr_append(elem_init->elems, elem_init->elems_count, elem);
		tok = peek_tok(parser);
		if (tok->kind != SCLEXER_SYMBOL || tok->data.symbol != SYM_COMMA)
			break;
		eat_tok(parser);
	}
	tok = eat_tok(parser);
	if (tok->kind != SCLEXER_SYMBOL || tok->data.symbol != SYM_BRACE_R)
		goto err_elem_init_not_end;
	return 0;
err_elem_init_not_end:
	print_err("element initialization not end", tok);
	return 1;
}

int
parse_expr_value(struct zako_value *value, struct parser *parser)
{
	struct sclexer_tok *tok;
	assert(value && parser);
	value->kind = EXPR_VALUE;
	value->data.expr = parse_expr(parser, true);
	if (!value->data.expr)
		return 1;
	tok = eat_tok(parser);
	if (tok->kind != SCLEXER_SYMBOL || tok->data.symbol != SYM_PAREN_R)
		goto err_expr_not_end;
	return 0;
err_expr_not_end:
	print_err("expression not end", tok);
	free_expr(value->data.expr);
	return 1;
}

int
parse_value_by_sclexer_symbol(
		struct sclexer_tok *tok,
		struct zako_value *value,
		struct parser *parser)
{
	assert(tok && value && parser);
	assert(tok->kind == SCLEXER_SYMBOL);
	switch (tok->data.symbol) {
	case SYM_BRACE_L:
		return parse_elem_init_value(value, parser);
	case SYM_PAREN_L:
		return parse_expr_value(value, parser);
	default:
		break;
	}
	printf_err("unexpected symbol '%s'",
			tok,
			cstr_symbol(tok->data.symbol));
	return 1;
}

int
parse_value_by_sclexer_ident(
		struct sclexer_tok *tok,
		struct zako_value *value,
		struct parser *parser)
{
	struct zako_ident *ident;
	char *ident_name;
	assert(tok && value && parser);
	assert(tok->kind == SCLEXER_IDENT);
	ident_name = dup_slice_to_cstr(&tok->data.str);
	value->kind = IDENT_VALUE;
	value->data.ident = ident = find_ident_in_scope(
			ident_name,
			parser->cur_scope);
	if (!value->data.ident)
		goto err_ident_not_found;

	free(ident_name);

	switch (ident->type->builtin) {
	case ARR_TYPE:
		return parse_arr_elem_value(value, ident, parser);
	case FN_TYPE:
		value->kind = FN_CALL_VALUE;
		value->data.fn_call = parse_fn_call(ident, parser);
		if (!value->data.fn_call)
			return 1;
		break;
	default:
		break;
	}

	return 0;
err_ident_not_found:
	printf_err("identifier '%s' not found", tok, ident_name);
	free(ident_name);
	return 1;
}

struct zako_value *
parse_value(struct parser *parser)
{
	struct zako_value *value;
	struct sclexer_tok *tok;
	assert(parser);
	tok = eat_tok(parser);
	value = ecalloc(1, sizeof(*value));

	if (tok->kind == SCLEXER_SYMBOL) {
		if (parse_value_by_sclexer_symbol(tok, value, parser))
			goto err_free_value;
		return value;
	} else if (tok->kind == SCLEXER_IDENT) {
		if (parse_value_by_sclexer_ident(tok, value, parser))
			goto err_free_value;
		return value;
	} else if (tok->kind != SCLEXER_INT && tok->kind != SCLEXER_INT_NEG) {
		goto err_free_value;
	}

	value->kind = INT_LITERAL;
	value->data.i = tok->data.sint;
	return value;
err_free_value:
	free(value);
	return NULL;
}
