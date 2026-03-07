/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "expr.h"
#include "fn.h"
#include "if.h"
#include "let.h"
#include "parser.h"
#include "return.h"
#include "sclexer.h"
#include "stmt.h"
#include "utils.h"
#include "while.h"
#include "../darr.h"
#include "../err.h"
#include "../lexer.h"

struct zako_stmt *
parse_stmt(struct sclexer_tok *tok, struct parser *parser)
{
	assert(tok && parser);

	if (tok->kind != SCLEXER_KEYWORD) {
		skip_white_tok(parser);
		return parse_expr_stmt(tok, parser);
	}

	eat_tok_skip_white(parser);
	switch (tok->data.keyword) {
	case KEYWORD_IF:
		return parse_if_stmt(parser);
	case KEYWORD_LET:
		return parse_let_stmt(parser);
	case KEYWORD_RETURN:
		return parse_return_stmt(parser);
	case KEYWORD_WHILE:
		return parse_while_stmt(parser);
	default:
		break;
	}
	printf_err("unexpected keyword '%s'",
			tok,
			cstr_keyword(tok->data.keyword));
	return NULL;
}

int
parse_stmt_block(
		struct zako_stmt ***_stmts,
		size_t *_stmts_count,
		struct parser *parser)
{
	struct zako_stmt *stmt;
	struct zako_stmt **stmts = NULL;
	size_t stmts_count = 0;
	struct sclexer_tok *tok;
	assert(_stmts && _stmts_count && parser);
	tok = eat_tok_skip_white(parser);
	if (tok->kind != SCLEXER_INDENT_BLOCK_BEGIN)
		goto err_block_begin_not_found;
	tok = peek_tok_skip_white(parser);
	while (tok->kind != SCLEXER_INDENT_BLOCK_END) {
		stmt = parse_stmt(tok, parser);
		if (!stmt)
			goto err_block_end_not_found;
		darr_append(stmts, stmts_count, stmt);
		tok = peek_tok_skip_white(parser);
	}
	eat_tok_skip_white(parser);
	*_stmts = stmts;
	*_stmts_count = stmts_count;
	return 0;
err_block_begin_not_found:
	print_err("block not begin", tok);
	return 1;
err_block_end_not_found:
	print_err("block not end", tok);
	return 1;
}

struct zako_toplevel_stmt *
parse_toplevel_stmt(struct sclexer_tok *tok, struct parser *parser)
{
	assert(tok && parser);
	/* Definition also handle declaration,
	 * but when declaration is correct and not have definition,
	 * fallback to declaration. */
	if (tok->kind == SCLEXER_IDENT) {
		return parse_fn_definition(tok, parser, false);
	} else if (tok->kind == SCLEXER_KEYWORD) {
		switch (tok->data.keyword) {
		case KEYWORD_PUB:
			tok = eat_tok(parser);
			return parse_fn_definition(tok, parser, true);
		default:
			break;
		}
	}
	print_err("unexpected token", tok);
	return NULL;
}
