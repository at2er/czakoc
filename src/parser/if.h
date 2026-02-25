/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_PARSER_IF_H
#define CZAKOC_PARSER_IF_H
#include "parser.h"
#include "../stmt.h"

struct zako_stmt *parse_if_stmt(struct parser *parser);

#endif
