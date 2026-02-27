/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdlib.h>
#include "expr.h"
#include "fn.h"
#include "if.h"
#include "let.h"
#include "return.h"
#include "stmt.h"

void
free_stmt(struct zako_stmt *self)
{
	if (!self)
		return;
	switch (self->kind) {
	case IF_STMT:
		free_if_stmt(self->inner.if_stmt);
		break;
	case LET_STMT:
		free_let_stmt(self->inner.let_stmt);
		break;
	case RETURN_STMT:
		free_return_stmt(self->inner.return_stmt);
		break;
	}
	free(self);
}

void
free_toplevel_stmt(struct zako_toplevel_stmt *self)
{
	if (!self)
		return;
	switch (self->kind) {
	case FN_DECLARATION:
		free_fn_declaration(self->inner.fn_declaration);
		break;
	case FN_DEFINITION:
		free_fn_definition(self->inner.fn_definition);
		break;
	}
	free(self);
}

void
print_stmt(struct zako_stmt *self, Jim *jim)
{
	Jim fallback = {.pp = JIM_PP};
	if (!self)
		return;
	if (!jim)
		jim = &fallback;
	switch (self->kind) {
	case IF_STMT:
		print_if_stmt(self->inner.if_stmt, jim);
		break;
	case LET_STMT:
		print_let_stmt(self->inner.let_stmt, jim);
		break;
	case RETURN_STMT:
		jim_object_begin(jim);
		jim_member_key(jim, "kind");
		jim_string(jim, "return stmt");
		jim_member_key(jim, "expr");
		print_expr(self->inner.return_stmt->expr, jim);
		jim_object_end(jim);
		break;
	}
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
