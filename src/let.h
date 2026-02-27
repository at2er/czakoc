/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_LET_H
#define CZAKOC_LET_H
#include <stddef.h>
#include "expr.h"
#include "ident.h"

struct zako_let_stmt {
	struct zako_expr *expr;
	struct zako_ident *ident;
};

void free_let_stmt(struct zako_let_stmt *self);
void print_let_stmt(struct zako_let_stmt *self, Jim *jim);

#endif
