/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdlib.h>
#include "fn.h"
#include "lexer.h"
#include "type.h"

void
free_zako_type(struct zako_type *self)
{
	if (!self)
		return;
	destory_zako_fn_type(&self->inner.fn);
	free(self);
}

void
print_type(struct zako_type *self, Jim *jim)
{
	Jim fallback = {.pp = JIM_PP};
	if (!jim)
		jim = &fallback;
	jim_object_begin(jim);
	jim_member_key(jim, "kind");
	jim_string(jim, "type");
	jim_member_key(jim, "builtin");
	if (self->builtin <= U64_TYPE) {
		jim_string(jim, cstr_keyword(KEYWORD_I8 + self->builtin));
	} else if (self->builtin == FN_TYPE) {
		jim_string(jim, "function");
	}
	jim_object_end(jim);
}
