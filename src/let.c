/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdlib.h>
#include "expr.h"
#include "ident.h"
#include "jim2.h"
#include "let.h"
#include "stmt.h"

void
free_let_stmt(struct zako_let_stmt *self)
{
	if (!self)
		return;
	/* self->ident will be freed by exit_scope() */
	free_expr(self->expr);
	free(self);
}

void
print_let_stmt(struct zako_let_stmt *self, Jim *jim)
{
	Jim fallback = {.pp = JIM_PP};
	if (!self)
		return;
	if (!jim)
		jim = &fallback;
	jim_object_begin(jim);
	jim_member_key(jim, "kind");
	jim_string(jim, "let stmt");
	jim_member_key(jim, "ident");
	print_ident(self->ident, jim);
	jim_member_key(jim, "expr");
	print_expr(self->expr, jim);
	jim_object_end(jim);
}
