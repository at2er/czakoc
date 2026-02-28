/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <limits.h>
#include <stdlib.h>
#include "fn.h"
#include "lexer.h"
#include "panic.h"
#include "type.h"

static void destroy_arr_type(struct zako_arr_type *self);

void
destroy_arr_type(struct zako_arr_type *self)
{
	if (!self)
		return;
}

void
free_type(struct zako_type *self)
{
	if (!self)
		return;
	switch (self->builtin) {
	CASE_NUMBERS:
		break;
	case ARR_TYPE:
		destroy_arr_type(&self->inner.arr);
	case CMP_EXPR_TYPE:
		break;
	case FN_TYPE:
		destroy_fn_type(&self->inner.fn);
	}
	free(self);
}

enum ZAKO_BUILTIN_TYPE
get_builtin_type_from_int_literal(int64_t i)
{
	if (i <= INT8_MAX)
		return I8_TYPE;
	if (i <= INT16_MAX)
		return I16_TYPE;
	if (i <= INT32_MAX)
		return I32_TYPE;
	if (i <= INT64_MAX)
		return I64_TYPE;
	panic("get_builtin_type_from_int_literal()");
	return -1;
}

void
print_type(struct zako_type *self, Jim *jim)
{
	Jim fallback = {.pp = JIM_PP};
	if (!self)
		return;
	if (!jim)
		jim = &fallback;
	jim_object_begin(jim);
	jim_member_key(jim, "kind");
	jim_string(jim, "type");
	jim_member_key(jim, "builtin");
	switch (self->builtin) {
	CASE_NUMBERS:
		jim_string(jim, cstr_keyword(KEYWORD_I8 + self->builtin));
		break;
	case ARR_TYPE:
		jim_string(jim, "array");
		jim_member_key(jim, "size");
		jim_integer(jim, self->inner.arr.size);
		jim_member_key(jim, "element type");
		print_type(self->inner.arr.elem_type, jim);
		break;
	case CMP_EXPR_TYPE:
		jim_string(jim, "comparison");
		break;
	case FN_TYPE:
		jim_string(jim, "function");
		break;
	}
	jim_object_end(jim);
}
