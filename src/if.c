/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdlib.h>
#include "if.h"
#include "stmt.h"

void
free_zako_if_stmt(struct zako_if_stmt *self)
{
	if (!self)
		return;
	for (size_t i = 0; i < self->stmts_count; i++)
		free_zako_stmt(self->stmts[i]);
	free(self->stmts);
	free(self);
}
