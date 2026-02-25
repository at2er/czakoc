/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_IF_H
#define CZAKOC_IF_H
#include <stddef.h>

struct zako_if_stmt {
	struct zako_stmt **stmts;
	size_t stmts_count;
};

void free_zako_if_stmt(struct zako_if_stmt *self);

#endif
