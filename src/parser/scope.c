/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include "parser.h"
#include "scope.h"
#include "../ealloc.h"
#include "../ident.h"

static void free_ident_in_scope(size_t idx, struct zako_scope *scope);

void
free_ident_in_scope(size_t idx, struct zako_scope *scope)
{
	if (!scope->fn)
		return;
	if (scope->idents[idx] == scope->fn->declaration->ident
			->type->inner.fn.args[idx])
		return;
	free_zako_ident(scope->idents[idx]);
}

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
	for (size_t i = 0; i < self->idents_count; i++)
		free_ident_in_scope(i, self);
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
