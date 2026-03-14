/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "czakoc.h"
#include "ealloc.h"
#include "module.h"
#include "str.h"

void
free_module(struct zako_module *mod)
{
	if (!mod)
		return;
	free(mod->prefix);
	free(mod);
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
