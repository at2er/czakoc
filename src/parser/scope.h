/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_PARSER_SCOPE_H
#define CZAKOC_PARSER_SCOPE_H
#include <stddef.h>
#include "fn.h"
#include "ident.h"

struct zako_scope {
	struct zako_ident **idents;
	size_t idents_count;

	struct zako_fn_definition *fn;
	struct zako_scope *parent;
};

struct zako_ident *find_ident_in_scope(
		const char *name,
		struct zako_scope *scope);

#endif
