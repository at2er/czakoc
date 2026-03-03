/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_PARSER_STMT_H
#define CZAKOC_PARSER_STMT_H
#include "parser.h"

struct zako_stmt *parse_stmt(
		struct sclexer_tok *tok,
		struct parser *parser);

int parse_stmt_block(
		struct zako_stmt ***_stmts,
		size_t *_stmts_count,
		struct parser *parser);

struct zako_toplevel_stmt *parse_toplevel_stmt(
		struct sclexer_tok *tok,
		struct parser *parser);

#endif
