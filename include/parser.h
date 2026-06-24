/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_PARSER_H
#define CZAKOC_PARSER_H
#include "darr.h"

struct zk_ast_root {
	darr(struct zk_top_stmt *) stmts;
};

int parse_file(const char *path);

#endif
