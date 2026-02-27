/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_PARSER_LET_H
#define CZAKOC_PARSER_LET_H
#include "parser.h"
#include "../stmt.h"

struct zako_stmt *parse_let_stmt(struct parser *parser);

#endif
