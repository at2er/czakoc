/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "expr.h"
#include "ident.h"
#include "return.h"
#include "scope.h"
#include "utils.h"
#include "../ealloc.h"
#include "../err.h"
#include "../return.h"
#include "../semantics/semantics.h"
#include "../stmt.h"

struct zako_stmt *
parse_return_stmt(struct parser *parser)
{
	struct zako_ident *cur_fn_ident;
	int ret;
	struct zako_return_stmt *self;
	struct zako_stmt *stmt;
	struct sclexer_tok *begin;
	assert(parser);
	begin = peek_tok(parser);
	stmt = ecalloc(1, sizeof(*stmt));
	stmt->kind = RETURN_STMT;
	stmt->inner.return_stmt = self = ecalloc(1, sizeof(*self));
	self->expr = parse_expr(parser, false);
	if (!self->expr)
		goto err_free_stmt;
	cur_fn_ident = parser->cur_scope->fn->declaration->ident;
	ret = analyse_expr(self->expr, cur_fn_ident->type->inner.fn.type);
	if (ret)
		goto err_analyse_expr;
	return stmt;
err_analyse_expr:
	printf_err("return stmt: %s", begin, cstr_analysis_result(ret));
err_free_stmt:
	free_zako_stmt(stmt);
	return NULL;
}
