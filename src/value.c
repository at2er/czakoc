/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdlib.h>
#include "expr.h"
#include "fn.h"
#include "ident.h"
#include "value.h"

void destroy_elem_init(struct zako_elem_init_value *self);
void print_elem_init_value(struct zako_elem_init_value *self, Jim *jim);

void
destroy_elem_init(struct zako_elem_init_value *self)
{
	if (!self)
		return;
	for (size_t i = 0; i < self->elems_count; i++)
		free_value(self->elems[i]);
	free(self->elems);
}

void
print_elem_init_value(struct zako_elem_init_value *self, Jim *jim)
{
	assert(jim);
	if (!self)
		return;
	jim_array_begin(jim);
	for (size_t i = 0; i < self->elems_count; i++)
		print_value(self->elems[i], jim);
	jim_array_end(jim);
}

void
free_value(struct zako_value *self)
{
	if (!self)
		return;
	switch (self->kind) {
	case ELEM_INIT_VALUE:
		destroy_elem_init(&self->data.elem_init);
		break;
	case EXPR_VALUE:
		free_expr(self->data.expr);
		break;
	case FN_CALL_VALUE:
		free_fn_call(self->data.fn_call);
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
	if (!self)
		return;
	if (!jim)
		jim = &fallback;
	jim_object_begin(jim);
	jim_member_key(jim, "kind");
	jim_string(jim, "value");
	switch (self->kind) {
	case ELEM_INIT_VALUE:
		jim_member_key(jim, "element initialization");
		print_elem_init_value(&self->data.elem_init, jim);
		break;
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
