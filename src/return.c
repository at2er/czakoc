/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdlib.h>
#include "expr.h"
#include "return.h"

void
free_return_stmt(struct zako_return_stmt *self)
{
	if (!self)
		return;
	free_expr(self->expr);
	free(self);
}
