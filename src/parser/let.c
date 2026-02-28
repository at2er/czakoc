/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include "expr.h"
#include "ident.h"
#include "let.h"
#include "parser.h"
#include "sclexer.h"
#include "scope.h"
#include "utils.h"
#include "../darr.h"
#include "../ealloc.h"
#include "../err.h"
#include "../let.h"
#include "../lexer.h"
#include "../semantics/expr.h"
#include "../semantics/semantics.h"

struct zako_stmt *
parse_let_stmt(struct parser *parser)
{
	struct sclexer_tok *begin, *tok;
	bool mutable = false;
	int ret;
	struct zako_let_stmt *self;
	struct zako_stmt *stmt;
	assert(parser);
	begin = eat_tok(parser);
	if (begin->kind == SCLEXER_KEYWORD && begin->data.keyword == KEYWORD_MUT) {
		mutable = true;
		begin = eat_tok(parser);
	}
	stmt = ecalloc(1, sizeof(*stmt));
	stmt->kind = LET_STMT;
	stmt->inner.let_stmt = self = ecalloc(1, sizeof(*self));
	self->ident = parse_ident_sign(begin, parser);
	if (!self->ident)
		goto err_free_stmt;
	self->ident->mutable = mutable;
	tok = eat_tok(parser);
	if (tok->kind != SCLEXER_SYMBOL || tok->data.symbol != SYM_ASSIGN)
		goto err_unexpected_token;
	self->expr = parse_expr(parser, false);
	if (!self->expr)
		goto err_free_stmt;
	ret = analyse_expr(self->expr, self->ident->type);
	if (ret)
		goto err_analyse_expr;
	darr_append(parser->cur_scope->idents,
			parser->cur_scope->idents_count,
			self->ident);
	return stmt;
err_unexpected_token:
	print_err("let stmt: unexpected token", tok);
	goto err_free_stmt;
err_analyse_expr:
	printf_err("let stmt: %s", tok, cstr_analysis_result(ret));
err_free_stmt:
	free_stmt(stmt);
	return NULL;
}
