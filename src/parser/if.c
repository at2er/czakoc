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

static int parse_body(struct zako_if_stmt *if_stmt, struct parser *parser);

int
parse_body(struct zako_if_stmt *if_stmt, struct parser *parser)
{
	struct zako_stmt *stmt;
	struct sclexer_tok *tok;
	assert(if_stmt && parser);
	tok = eat_tok(parser);
	if (tok->kind != SCLEXER_KEYWORD || tok->data.keyword != KEYWORD_THEN)
		goto err_unexpected_token;
	tok = peek_tok(parser);
	if (tok->kind == SCLEXER_EOL) {
		tok = eat_tok(parser);
	} else if (tok->kind != SCLEXER_INDENT_BLOCK_BEGIN) {
		eat_tok(parser);
		stmt = parse_stmt(tok, parser);
		if (!stmt)
			return 1;
		darr_append(if_stmt->stmts, if_stmt->stmts_count, stmt);
		tok = eat_tok(parser);
		return 0;
	}
	if (tok->kind != SCLEXER_INDENT_BLOCK_BEGIN)
		goto err_unexpected_token;
	return 0;
err_unexpected_token:
	print_err("unexpected token", tok);
	return 1;
}

struct zako_stmt *
parse_if_stmt(struct parser *parser)
{
	struct zako_if_stmt *self;
	struct zako_stmt *stmt;
	assert(parser);
	stmt = ecalloc(1, sizeof(*stmt));
	stmt->kind = IF_STMT;
	stmt->inner.if_stmt = self = ecalloc(1, sizeof(*self));
	self->cond = parse_expr(parser, false);
	if (parse_body(self, parser))
		goto err_free_stmt;
	return stmt;
err_free_stmt:
	free_zako_stmt(stmt);
	return NULL;
}
