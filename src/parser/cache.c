/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include <errno.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "cache.h"
#include "../czakoc.h"
#include "../ealloc.h"
#include "../err.h"
#include "../fn.h"
#include "../mkdirp.h"
#include "../panic.h"
#include "../path_max.h"
#include "../str.h"

static void cache_fn_declaration(
		struct zako_toplevel_stmt *stmt,
		FILE *stream);

void
cache_fn_declaration(struct zako_toplevel_stmt *stmt, FILE *stream)
{
	char *buf;
	struct zako_fn_declaration *dec;
	int len;
	assert(stmt && stream);

	if (stmt->kind == FN_DEFINITION)
		dec = stmt->inner.fn_definition->declaration;
	else
		dec = stmt->inner.fn_declaration;

	if (dec->public == false)
		return;

	len = dec->end_at - stmt->begin;
	buf = ecalloc(len + 1, sizeof(char));

	snprintf(buf, len, "%.*s", len, stmt->begin);
	buf[len - 1] = '\n';

	fputs(buf, stream);
	free(buf);
}

int
cache_file(struct zako_toplevel_stmt **stmts,
		size_t stmts_count,
		struct zako_module *mod)
{
	char *path, *path_cpy;
	FILE *stream;

	assert(stmts && mod);
	assert(*stmts);

	path = get_cache(mod->file_path);
	if (!path)
		return 0;

	if (has_cache(path, mod->file_path)) {
		free(path);
		return 0;
	}

	path_cpy = strdup(path);
	if (mkdirp(dirname(path_cpy), czakoc_file_mode))
		panic("mkdirp()");
	free(path_cpy);

	stream = fopen(path, "w");
	for (size_t i = 0; i < stmts_count; i++) {
		switch (stmts[i]->kind) {
		case FN_DECLARATION:
			cache_fn_declaration(stmts[i], stream);
			break;
		case FN_DEFINITION:
			cache_fn_declaration(stmts[i], stream);
			break;
		default:
			break;
		}
	}
	fclose(stream);
	free(path);

	return 0;
}

int
create_cache_dir(void)
{
	if (!czakoc_cache_dir)
		return 0;

	if (mkdirp(czakoc_cache_dir, czakoc_file_mode))
		goto err_no_such_file_or_dir;

	return 0;
err_no_such_file_or_dir:
	printf_err_msg("no such file or directory: %s", czakoc_cache_dir);
	return 1;
}

char *
get_cache(const char *path)
{
	char *cwd_real, *path_real;
	struct str result;

	if (!czakoc_cache_dir)
		return NULL;

	assert(path);

	cwd_real = realpath(czakoc_cwd, NULL);
	path_real = realpath(path, NULL);

	estr_from_cstr(&result, czakoc_cache_dir);
	estr_append_chr(&result, '/');
	estr_append_cstr(&result, &path_real[strlen(cwd_real) + 1]);

	free(cwd_real);
	free(path_real);

	return result.s;
}

bool
has_cache(const char *cache, const char *src)
{
	struct stat cache_st, src_st;

	assert(src);
	if (!cache)
		return false;

	if (stat(cache, &cache_st) != 0)
		return false;
	if (S_ISDIR(cache_st.st_mode))
		panicf("%s is directory", cache);

	if (czakoc_flags & CZAKOC_FORCE_BUILD)
		return false;

	if (stat(src, &src_st) != 0)
		panic("stat()");

	if (cache_st.st_mtime < src_st.st_mtime)
		return false;
	return true;
}
