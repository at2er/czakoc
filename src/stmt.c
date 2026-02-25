/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdlib.h>
#include "expr.h"
#include "fn.h"
#include "if.h"
#include "return.h"
#include "stmt.h"

void
free_zako_stmt(struct zako_stmt *self)
{
	if (!self)
		return;
	switch (self->kind) {
	case IF_STMT:
		free_zako_if_stmt(self->inner.if_stmt);
		break;
	case RETURN_STMT:
		free_zako_return_stmt(self->inner.return_stmt);
		break;
	}
	free(self);
}

void
free_zako_toplevel_stmt(struct zako_toplevel_stmt *self)
{
	if (!self)
		return;
	switch (self->kind) {
	case FN_DECLARATION:
		free_zako_fn_declaration(self->inner.fn_declaration);
		break;
	case FN_DEFINITION:
		free_zako_fn_definition(self->inner.fn_definition);
		break;
	}
	free(self);
}

void
print_stmt(struct zako_stmt *self, Jim *jim)
{
	Jim fallback = {.pp = JIM_PP};
	if (!jim)
		jim = &fallback;
	jim_object_begin(jim);
	jim_member_key(jim, "kind");
	switch (self->kind) {
	case IF_STMT:
		jim_string(jim, "if");
		// TODO
		break;
	case RETURN_STMT:
		jim_string(jim, "return");
		jim_member_key(jim, "expr");
		print_expr(self->inner.return_stmt->expr, jim);
		break;
	}
	jim_object_end(jim);
}

void
print_toplevel_stmt(struct zako_toplevel_stmt *self, Jim *jim)
{
	Jim fallback = {.pp = JIM_PP};
	if (!jim)
		jim = &fallback;
	switch (self->kind) {
	case FN_DECLARATION:
		return;
	case FN_DEFINITION:
		print_fn_definition(self->inner.fn_definition, jim);
		break;
	}
}
