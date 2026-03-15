/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include "module.h"
#include "sclexer.h"
#include "utils.h"
#include "../czakoc.h"
#include "../darr.h"
#include "../ealloc.h"
#include "../err.h"
#include "../lexer.h"
#include "../module.h"
#include "../str.h"

struct zako_toplevel_stmt *
parse_module_import(
		struct sclexer_tok *tok,
		struct parser *parser)
{
	struct str path, buf;
	struct zako_module_import *self;
	struct zako_toplevel_stmt *stmt = NULL;
	assert(tok && parser);

	tok = eat_tok(parser);
	if (tok->kind != SCLEXER_IDENT)
		goto err_miss_name;

	stmt = ecalloc(1, sizeof(*stmt));
	stmt->kind = MODULE_IMPORT;
	stmt->inner.mod_import = self = ecalloc(1, sizeof(*self));
	self->name = dup_slice_to_cstr(&tok->data.str);

	tok = eat_tok(parser);
	if (tok->kind != SCLEXER_SYMBOL || tok->data.symbol != SYM_ASSIGN)
		goto err_not_assign;
	tok = eat_tok(parser);
	if (tok->kind != SCLEXER_IDENT)
		goto err_miss_path;

	estr_from_cstr(&path, czakoc_cwd);
	estr_append_chr(&path, '/');
	str_from_sclexer_str_slice(&buf, &tok->data.str);
	estr_append_str(&path, &buf);
	tok = peek_tok_skip_white(parser);
	while (tok->kind == SCLEXER_SYMBOL && tok->data.symbol == SYM_DOT) {
		eat_tok_skip_white(parser);
		tok = eat_tok(parser);
		if (tok->kind != SCLEXER_IDENT)
			goto err_miss_path;
		estr_append_chr(&path, '/');
		str_from_sclexer_str_slice(&buf, &tok->data.str);
		estr_append_str(&path, &buf);
		tok = peek_tok_skip_white(parser);
	}

	estr_append_cstr(&path, ".zako");
	self->mod = parse_file(path.s);

	darr_append(parser->imported, parser->imported_count, self);

	str_free(&path);
	return stmt;
err_miss_name:
	print_err("miss module name", tok);
	goto err_free_stmt;
err_not_assign:
	print_err("miss '='", tok);
	goto err_free_stmt;
err_miss_path:
	print_err("miss path identifier", tok);
err_free_stmt:
	free_toplevel_stmt(stmt);
	return NULL;
}
