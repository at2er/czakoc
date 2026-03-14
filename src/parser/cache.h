/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_PARSER_CACHE_H
#define CZAKOC_PARSER_CACHE_H
#include <stdbool.h>
#include "../module.h"
#include "../stmt.h"

int cache_file(struct zako_toplevel_stmt **stmts,
		size_t stmts_count,
		struct zako_module *mod);
int create_cache_dir(void);
char *get_cache(const char *path);

#endif
