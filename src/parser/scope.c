/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include "parser.h"
#include "scope.h"
#include "../ealloc.h"
#include "../ident.h"

void
enter_scope(struct parser *parser)
{
	struct zako_scope *scope;
	assert(parser);
	scope = ecalloc(1, sizeof(*scope));
	if (parser->cur_scope)
		scope->parent = parser->cur_scope;
	parser->cur_scope = scope;
}

void
exit_scope(struct parser *parser)
{
	struct zako_scope *self;
	assert(parser);
	assert(parser->cur_scope);
	self = parser->cur_scope;
	parser->cur_scope = parser->cur_scope->parent;
	/* elements of self->idents will be freed by parser. */
	free(self->idents);
	free(self);
}

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
