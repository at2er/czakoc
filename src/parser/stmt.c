/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "fn.h"
#include "if.h"
#include "let.h"
#include "parser.h"
#include "return.h"
#include "sclexer.h"
#include "stmt.h"
#include "utils.h"
#include "../err.h"
#include "../lexer.h"

struct zako_stmt *
parse_stmt(struct sclexer_tok *tok, struct parser *parser)
{
	assert(tok && parser);
	if (tok->kind != SCLEXER_KEYWORD)
		return NULL; /* expression statement */
	switch (tok->data.keyword) {
	case KEYWORD_IF:
		return parse_if_stmt(parser);
	case KEYWORD_LET:
		return parse_let_stmt(parser);
	case KEYWORD_RETURN:
		return parse_return_stmt(parser);
	default:
		break;
	}
	printf_err("unexpected keyword '%s'",
			tok,
			cstr_keyword(tok->data.keyword));
	return NULL;
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
