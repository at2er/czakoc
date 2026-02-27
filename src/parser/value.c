/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "expr.h"
#include "parser.h"
#include "scope.h"
#include "utils.h"
#include "value.h"
#include "../ealloc.h"
#include "../err.h"
#include "../lexer.h"
#include "../value.h"

struct zako_value *
parse_value(struct parser *parser)
{
	char *ident_name;
	struct zako_value *value;
	struct sclexer_tok *tok;
	assert(parser);
	tok = eat_tok(parser);
	value = ecalloc(1, sizeof(*value));

	if (tok->kind == SCLEXER_SYMBOL && tok->data.symbol == SYM_PAREN_L) {
		value->kind = EXPR_VALUE;
		value->data.expr = parse_expr(parser, true);
		if (!value->data.expr)
			goto err_free_value;
		tok = eat_tok(parser);
		if (tok->kind != SCLEXER_SYMBOL || tok->data.symbol != SYM_PAREN_R)
			goto err_expr_not_end;
		return value;
	} else if (tok->kind == SCLEXER_IDENT) {
		ident_name = dup_slice_to_cstr(&tok->data.str);
		value->kind = IDENT_VALUE;
		value->data.ident = find_ident_in_scope(
				ident_name,
				parser->cur_scope);
		if (!value->data.ident)
			goto err_ident_not_found;

		free(ident_name);
		if (value->data.ident->type->builtin == FN_TYPE) {
			value->kind = FN_CALL_VALUE;
			value->data.fn_call = parse_fn_call(
					value->data.ident,
					parser);
			if (!value->data.fn_call)
				goto err_free_value;
		}

		return value;
	} else if (tok->kind != SCLEXER_INT && tok->kind != SCLEXER_INT_NEG) {
		goto err_free_value;
	}

	value->kind = INT_LITERAL;
	value->data.i = tok->data.sint;
	return value;
err_expr_not_end:
	print_err("expression not end", tok);
	free_zako_expr(value->data.expr);
	goto err_free_value;
err_ident_not_found:
	printf_err("identifier '%s' not found", tok, ident_name);
	free(ident_name);
err_free_value:
	free(value);
	return NULL;
}
