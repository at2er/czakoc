/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_PARSER_EXPR_H
#define CZAKOC_PARSER_EXPR_H
#include <stdbool.h>
#include "parser.h"
#include "../expr.h"
#include "../stmt.h"

struct zako_expr *parse_cmp_expr(struct parser *parser, bool in_paren);
struct zako_expr *parse_expr(struct parser *parser, bool in_paren);
struct zako_stmt *parse_expr_stmt(struct sclexer_tok *tok, struct parser *parser);

#endif
