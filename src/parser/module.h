/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_PARSER_MODULE_H
#define CZAKOC_PARSER_MODULE_H
#include "parser.h"
#include "../stmt.h"

struct zako_toplevel_stmt *parse_module_import(
		struct sclexer_tok *tok,
		struct parser *parser);

#endif
