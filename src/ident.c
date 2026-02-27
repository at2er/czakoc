/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdlib.h>
#include "ident.h"

void
free_ident(struct zako_ident *self)
{
	free(self->name);
	free_type(self->type);
	free(self);
}

void
print_ident(struct zako_ident *self, Jim *jim)
{
	Jim fallback = {.pp = JIM_PP};
	if (!self)
		return;
	if (!jim)
		jim = &fallback;
	jim_object_begin(jim);
	jim_member_key(jim, "kind");
	jim_string(jim, "identifier");
	jim_member_key(jim, "name");
	jim_string(jim, self->name);
	jim_member_key(jim, "type");
	print_type(self->type, jim);
	jim_object_end(jim);
}
