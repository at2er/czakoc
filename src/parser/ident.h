/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_PARSER_IDENT_H
#define CZAKOC_PARSER_IDENT_H
#include "parser.h"
#include "sclexer.h"
#include "../ident.h"

struct zako_ident *parse_ident_sign(
		struct sclexer_tok *tok,
		struct parser *parser);

#endif
