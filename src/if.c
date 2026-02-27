/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdlib.h>
#include "if.h"
#include "stmt.h"

void
free_zako_if_stmt(struct zako_if_stmt *self)
{
	if (!self)
		return;
	free_zako_expr(self->cond);
	for (size_t i = 0; i < self->stmts_count; i++)
		free_zako_stmt(self->stmts[i]);
	free(self->stmts);
	free(self);
}

void
print_if_stmt(struct zako_if_stmt *self, Jim *jim)
{
	Jim fallback = {.pp = JIM_PP};
	if (!self)
		return;
	if (!jim)
		jim = &fallback;
	jim_object_begin(jim);
	jim_member_key(jim, "kind");
	jim_string(jim, "if stmt");
	jim_member_key(jim, "cond");
	print_expr(self->cond, jim);
	jim_member_key(jim, "stmts_count");
	jim_integer(jim, self->stmts_count);
	jim_member_key(jim, "stmts");
	for (size_t i = 0; i < self->stmts_count; i++)
		print_stmt(self->stmts[i], jim);
	jim_object_end(jim);
}
