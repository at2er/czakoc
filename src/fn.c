/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdlib.h>
#include "fn.h"
#include "ident.h"
#include "type.h"
#include "value.h"

void
destory_zako_fn_type(struct zako_fn_type *self)
{
	for (int i = 0; i < self->argc; i++)
		free_zako_ident(self->args[i]);
	free(self->args);
}

void
free_zako_fn_call(struct zako_fn_call *self)
{
	if (!self)
		return;
	for (int i = 0; i < self->argc; i++)
		free_zako_value(self->args[i]);
	free(self);
}

void
free_zako_fn_declaration(struct zako_fn_declaration *self)
{
	if (!self)
		return;
	free_zako_ident(self->ident);
	free(self);
}

void
free_zako_fn_definition(struct zako_fn_definition *self)
{
	if (!self)
		return;
	for (size_t i = 0; i < self->stmts_count; i++)
		free_zako_stmt(self->stmts[i]);
	free(self->stmts);
	free(self);
}

void
print_fn_call(struct zako_fn_call *self, Jim *jim)
{
	Jim fallback = {.pp = JIM_PP};
	if (!jim)
		jim = &fallback;
	jim_object_begin(jim);
	jim_member_key(jim, "kind");
	jim_string(jim, "function call");
	jim_member_key(jim, "args");
	jim_array_begin(jim);
	for (int i = 0; i < self->argc; i++)
		print_value(self->args[i], jim);
	jim_array_end(jim);
	jim_object_end(jim);
}

void
print_fn_definition(struct zako_fn_definition *self, Jim *jim)
{
	Jim fallback = {.pp = JIM_PP};
	struct zako_fn_type *fn_type;
	if (!jim)
		jim = &fallback;
	jim_object_begin(jim);
	jim_member_key(jim, "kind");
	jim_string(jim, "function definition");
	fn_type = &self->declaration->ident->type->inner.fn;
	jim_member_key(jim, "type");
	print_type(fn_type->type, jim);
	jim_member_key(jim, "args");
	jim_array_begin(jim);
	for (int i = 0; i < fn_type->argc; i++)
		print_ident(fn_type->args[i], jim);
	jim_array_end(jim);
	jim_member_key(jim, "statements");
	jim_array_begin(jim);
	for (size_t i = 0; i < self->stmts_count; i++)
		print_stmt(self->stmts[i], jim);
	jim_array_end(jim);
	jim_object_end(jim);
}
