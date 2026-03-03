/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_WHILE_H
#define CZAKOC_WHILE_H
#include <stddef.h>
#include "expr.h"

struct zako_while_stmt {
	struct zako_expr *cond;
	struct zako_stmt **stmts;
	size_t stmts_count;
};

void free_while_stmt(struct zako_while_stmt *self);
void print_while_stmt(struct zako_while_stmt *self, Jim *jim);

#endif
