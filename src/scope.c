/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "ident.h"
#include "scope.h"

struct zako_ident *
find_ident_in_scope(const char *name, struct zako_scope *scope)
{
	while (scope) {
		for (size_t i = 0; i < scope->idents_count; i++) {
			if (strcmp(name, scope->idents[i]->name) == 0)
				return scope->idents[i];
		}
		scope = scope->parent;
	}
	return NULL;
}

void
free_scope(struct zako_scope *self)
{
	if (!self)
		return;
	/* elements of self->idents will be freed by parser. */
	free(self->idents);
	free(self);
}
