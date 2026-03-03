/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_PARSER_WHILE_H
#define CZAKOC_PARSER_WHILE_H
#include "parser.h"
#include "../stmt.h"

struct zako_stmt *parse_while_stmt(struct parser *parser);

#endif
