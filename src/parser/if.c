/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "expr.h"
#include "if.h"
#include "sclexer.h"
#include "stmt.h"
#include "utils.h"
#include "../darr.h"
#include "../ealloc.h"
#include "../err.h"
#include "../expr.h"
#include "../if.h"
#include "../semantics/if.h"
#include "../semantics/semantics.h"

static int parse_if_body(struct zako_if_stmt *if_stmt, struct parser *parser);

int
parse_if_body(struct zako_if_stmt *if_stmt, struct parser *parser)
{
	struct zako_stmt *stmt;
	struct sclexer_tok *tok;
	assert(if_stmt && parser);
	tok = eat_tok(parser);
	if (tok->kind != SCLEXER_KEYWORD || tok->data.keyword != KEYWORD_THEN)
		goto err_unexpected_token;
	tok = eat_tok_skip_white(parser);
	if (tok->kind != SCLEXER_INDENT_BLOCK_BEGIN)
		goto err_block_begin_not_found;
	tok = peek_tok(parser);
	while (tok->kind != SCLEXER_INDENT_BLOCK_END) {
		eat_tok_skip_white(parser);
		stmt = parse_stmt(tok, parser);
		if (!stmt)
			goto err_block_end_not_found;
		darr_append(if_stmt->stmts, if_stmt->stmts_count, stmt);
		tok = peek_tok_skip_white(parser);
	}
	eat_tok_skip_white(parser);
	return 0;
err_unexpected_token:
	print_err("unexpected token", tok);
	return 1;
err_block_begin_not_found:
	print_err("block not begin", tok);
	return 1;
err_block_end_not_found:
	print_err("block not end", tok);
	return 1;
}

struct zako_stmt *
parse_if_stmt(struct parser *parser)
{
	struct sclexer_tok *begin;
	int ret;
	struct zako_if_stmt *self;
	struct zako_stmt *stmt;
	assert(parser);
	begin = peek_tok(parser);
	stmt = ecalloc(1, sizeof(*stmt));
	stmt->kind = IF_STMT;
	stmt->inner.if_stmt = self = ecalloc(1, sizeof(*self));
	self->cond = parse_cmp_expr(parser, false);
	if (self->cond->kind != CMP_EXPR)
		goto err_not_cmp;
	if (parse_if_body(self, parser))
		goto err_free_stmt;
	ret = analyse_if_stmt(self);
	if (ret)
		goto err_analyse_if_stmt;
	return stmt;
err_not_cmp:
	print_err("not comparison expression", begin);
	goto err_free_stmt;
err_analyse_if_stmt:
	printf_err("if stmt: %s", begin, cstr_analysis_result(ret));
err_free_stmt:
	free_zako_stmt(stmt);
	return NULL;
}
