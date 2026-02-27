/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_PARSER_FN_H
#define CZAKOC_PARSER_FN_H
#include <stdbool.h>
#include "parser.h"
#include "sclexer.h"
#include "../fn.h"
#include "../stmt.h"

struct zako_fn_call *parse_fn_call(
		struct zako_ident *callee,
		struct parser *parser);
struct zako_toplevel_stmt *parse_fn_definition(
		struct sclexer_tok *tok,
		struct parser *parser,
		bool public);
int parse_fn_sign(struct zako_fn_type *type, struct parser *parser);

#endif
