/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_RETURN_H
#define CZAKOC_RETURN_H

struct zako_return_stmt {
	struct zako_expr *expr;
};

void free_zako_return_stmt(struct zako_return_stmt *self);

#endif
