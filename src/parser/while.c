/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "expr.h"
#include "while.h"
#include "sclexer.h"
#include "stmt.h"
#include "utils.h"
#include "../darr.h"
#include "../ealloc.h"
#include "../err.h"
#include "../expr.h"
#include "../while.h"
#include "../semantics/expr.h"
#include "../semantics/semantics.h"

struct zako_stmt *
parse_while_stmt(struct parser *parser)
{
	struct sclexer_tok *begin;
	int ret;
	struct zako_while_stmt *self;
	struct zako_stmt *stmt;
	struct sclexer_tok *tok;
	assert(parser);
	begin = peek_tok(parser);
	stmt = ecalloc(1, sizeof(*stmt));
	stmt->kind = WHILE_STMT;
	stmt->inner.while_stmt = self = ecalloc(1, sizeof(*self));
	self->cond = parse_cmp_expr(parser, false);
	if (self->cond->kind != CMP_EXPR)
		goto err_not_cmp;
	ret = analyse_cmp_expr(self->cond);
	if (ret)
		goto err_analyse_cmp_expr;

	tok = eat_tok(parser);
	if (tok->kind != SCLEXER_KEYWORD || tok->data.keyword != KEYWORD_THEN)
		goto err_unexpected_token;
	if (parse_stmt_block(&self->stmts, &self->stmts_count, parser))
		goto err_free_stmt;

	return stmt;
err_not_cmp:
	print_err("while stmt: not comparison expression", begin);
	goto err_free_stmt;
err_analyse_cmp_expr:
	printf_err("while stmt: %s", begin, cstr_analysis_result(ret));
	goto err_free_stmt;
err_unexpected_token:
	print_err("unexpected token", tok);
err_free_stmt:
	free_stmt(stmt);
	return NULL;
}
