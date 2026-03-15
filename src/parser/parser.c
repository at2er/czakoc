/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include "cache.h"
#include "parser.h"
#include "sclexer.h"
#include "scope.h"
#include "stmt.h"
#include "utils.h"
#include "../compiler/compiler.h"
#include "../czakoc.h"
#include "../darr.h"
#include "../ealloc.h"
#include "../err.h"
#include "../jim2.h"
#include "../lexer.h"
#include "../panic.h"
#include "../stmt.h"

static void print_ast_by_jim(
		struct zako_toplevel_stmt **stmts,
		size_t stmts_count);

void
print_ast_by_jim(
		struct zako_toplevel_stmt **stmts,
		size_t stmts_count)
{
	Jim jim = {.pp = JIM_PP};
	jim_array_begin(&jim);
	for (size_t i = 0; i < stmts_count; i++)
		print_toplevel_stmt(stmts[i], &jim);
	jim_array_end(&jim);
	fwrite(jim.sink, jim.sink_count, 1, stdout);
	fwrite("\n", sizeof(char), 1, stdout);
}

struct zako_module *
find_module(const char *name, struct parser *parser)
{
	for (int i = 0; i < parser->imported_count; i++) {
		if (strcmp(name, parser->imported[i]->name) == 0)
			return parser->imported[i]->mod;
	}
	return NULL;
}

struct zako_module *
parse_file(const char *path)
{
	char *cache_path;
	struct sclexer_tok *cur;
	bool _has_cache = false;
	struct sclexer lexer = {0};
	struct parser parser = {0};
	char *src;
	struct zako_toplevel_stmt *stmt;
	struct zako_toplevel_stmt **stmts = NULL;
	size_t stmts_count = 0;
	assert(path);

	enter_scope(&parser);

	parser.mod = create_module(path);

	cache_path = get_cache(path);
	if (!(czakoc_flags & CZAKOC_FORCE_BUILD)) {
		_has_cache = has_cache(cache_path, path);
		if (_has_cache)
			path = cache_path;
	}

	src = init_lexer(&lexer, path);

	parser.tokens_count = sclexer_get_tokens(&lexer, &parser.tokens);
	if (czakoc_flags & CZAKOC_OUTPUT_LEXER_TOKENS) {
		for (size_t i = 0; i < parser.tokens_count; i++)
			sclexer_print_tok(&lexer, &parser.tokens[i]);
	}

	while (parser.cur_tok < parser.tokens_count) {
		cur = eat_tok(&parser);
		if (!cur)
			break;
		switch (cur->kind) {
		case SCLEXER_EOF:
			goto end;
		case SCLEXER_EOL:
			continue;
		case SCLEXER_IDENT:
		case SCLEXER_KEYWORD:
			stmt = parse_toplevel_stmt(cur, &parser);
			if (!stmt)
				goto err_free_all;
			darr_append(stmts, stmts_count, stmt);
			break;
		default:
			goto err_unknown_token;
		}
	}
end:
	if (czakoc_flags & CZAKOC_OUTPUT_AST)
		print_ast_by_jim(stmts, stmts_count);
	if (compile_file(stmts, stmts_count, parser.mod))
		goto err_compile_file;
	if (!_has_cache) {
		if (cache_file(stmts, stmts_count, parser.mod))
			panic("cache_file()");
	}

	exit_scope(&parser);
	for (size_t i = 0; i < stmts_count; i++)
		free_toplevel_stmt(stmts[i]);
	free(stmts);
	free(parser.tokens);

	free(src);
	return parser.mod;
err_unknown_token:
	print_err("unkown token", cur);
	goto err_free_all;
err_compile_file:
	printf_err_msg("compile file '%s'", path);
err_free_all:
	exit_scope(&parser);
	for (size_t i = 0; i < stmts_count; i++)
		free_toplevel_stmt(stmts[i]);
	free(stmts);
	free(parser.tokens);
	free(src);
	free_module(parser.mod);
	return NULL;
}
