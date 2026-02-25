/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_PARSER_H
#define CZAKOC_PARSER_H
#include <stddef.h>
#include "sclexer.h"
#include "../module.h"

struct zako_scope;
struct parser {
	size_t cur_tok;
	struct sclexer_tok *tokens;
	size_t tokens_count;

	struct zako_module *mod;

	struct zako_scope *cur_scope;
};

struct zako_module *parse_file(const char *path);

#endif
