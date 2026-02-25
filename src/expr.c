/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "expr.h"

void
free_zako_expr(struct zako_expr *self)
{
	if (!self)
		return;
	switch (self->kind) {
	case BINARY_EXPR:
		free_zako_value(self->inner.binary.lhs);
		free_zako_value(self->inner.binary.rhs);
		break;
	case PRIMARY_EXPR:
		free_zako_value(self->inner.primary);
		break;
	}
	free(self);
}

void
print_expr(struct zako_expr *self, Jim *jim)
{
	Jim fallback = {.pp = JIM_PP};
	if (!jim)
		jim = &fallback;
	jim_object_begin(jim);
	jim_member_key(jim, "kind");
	switch (self->kind) {
	case BINARY_EXPR:
		jim_string(jim, "binary");
		jim_member_key(jim, "op");
		jim_string(jim, cstr_symbol(self->inner.binary.op));
		jim_member_key(jim, "lhs");
		print_value(self->inner.binary.lhs, jim);
		jim_member_key(jim, "rhs");
		print_value(self->inner.binary.rhs, jim);
		break;
	case PRIMARY_EXPR:
		jim_string(jim, "primary");
		jim_member_key(jim, "value");
		print_value(self->inner.primary, jim);
		break;
	}
	jim_member_key(jim, "type");
	print_type(self->type, jim);
	jim_object_end(jim);
}
