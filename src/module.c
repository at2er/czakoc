/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "czakoc.h"
#include "ealloc.h"
#include "jim2.h"
#include "module.h"
#include "scope.h"
#include "str.h"

struct zako_module *
create_module(const char *path)
{
	struct zako_module *self;
	self = ecalloc(1, sizeof(*self));
	self->prefix = gen_mod_prefix(path);
	self->scope = ecalloc(1, sizeof(*self->scope));
	strcpy(self->file_path, path);
	return self;
}

void
free_module(struct zako_module *self)
{
	if (!self)
		return;
	free(self->prefix);
	for (size_t i = 0; i < self->scope->idents_count; i++)
		free_ident(self->scope->idents[i]);
	free(self);
}

void
free_module_import(struct zako_module_import *self)
{
	if (!self)
		return;
	free(self);
}

char *
gen_mod_prefix(const char *path)
{
	const char *begin, *stop;
	char *cwd_real = realpath(czakoc_cwd, NULL);
	char *path_real = realpath(path, NULL);
	struct str result;

	assert(path);
	
	/* it has a '/' at begin[0] */
	begin = &path_real[strlen(cwd_real)];

	estr_from_cstr(&result, czakoc_root_mod);

	stop = strrchr(begin, '.');
	for (const char *cur = begin; cur != stop && *cur; cur++) {
		if (*cur == '/' || *cur == '.') {
			estr_append_cstr(&result, "__");
			continue;
		}
		estr_append_chr(&result, *cur);
	}
	estr_append_cstr(&result, "__");

	free(cwd_real);
	free(path_real);

	return result.s;
}

void
print_module_import(struct zako_module_import *self, Jim *jim)
{
	Jim fallback = {.pp = JIM_PP};
	if (!self)
		return;
	if (!jim)
		jim = &fallback;
	jim_object_begin(jim);
	jim_member_key(jim, "name");
	jim_string(jim, self->name);
	jim_object_end(jim);
}
