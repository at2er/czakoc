/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_IF_H
#define CZAKOC_IF_H
#include <stddef.h>
#include "expr.h"

struct zako_if_stmt {
	struct zako_expr *cond;
	struct zako_stmt **stmts;
	size_t stmts_count;
};

void free_zako_if_stmt(struct zako_if_stmt *self);
void print_if_stmt(struct zako_if_stmt *self, Jim *jim);

#endif
