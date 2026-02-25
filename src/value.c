/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdlib.h>
#include "expr.h"
#include "fn.h"
#include "ident.h"
#include "value.h"

void
free_zako_value(struct zako_value *self)
{
	if (!self)
		return;
	switch (self->kind) {
	case EXPR_VALUE:
		free_zako_expr(self->data.expr);
		break;
	case FN_CALL_VALUE:
		free_zako_fn_call(self->data.fn_call);
		break;
	case IDENT_VALUE:
	case INT_LITERAL:
		break;
	}
	free(self);
}

void
print_value(struct zako_value *self, Jim *jim)
{
	Jim fallback = {.pp = JIM_PP};
	if (!jim)
		jim = &fallback;
	jim_object_begin(jim);
	jim_member_key(jim, "kind");
	jim_string(jim, "value");
	switch (self->kind) {
	case EXPR_VALUE:
		jim_member_key(jim, "expr");
		print_expr(self->data.expr, jim);
		break;
	case FN_CALL_VALUE:
		jim_member_key(jim, "function call");
		print_fn_call(self->data.fn_call, jim);
		break;
	case IDENT_VALUE:
		jim_member_key(jim, "ident");
		print_ident(self->data.ident, jim);
		break;
	case INT_LITERAL:
		jim_member_key(jim, "int");
		jim_integer(jim, self->data.i);
		break;
	}
	jim_member_key(jim, "type");
	print_type(self->type, jim);
	jim_object_end(jim);
}
