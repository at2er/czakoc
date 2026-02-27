/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <limits.h>
#include <stdlib.h>
#include "fn.h"
#include "lexer.h"
#include "panic.h"
#include "type.h"

void
free_type(struct zako_type *self)
{
	if (!self)
		return;
	if (self->builtin == FN_TYPE)
		destroy_zako_fn_type(&self->inner.fn);
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
	if (self->builtin <= U64_TYPE) {
		jim_string(jim, cstr_keyword(KEYWORD_I8 + self->builtin));
	} else if (self->builtin == CMP_EXPR_TYPE) {
		jim_string(jim, "comparison");
	} else if (self->builtin == FN_TYPE) {
		jim_string(jim, "function");
	}
	jim_object_end(jim);
}
