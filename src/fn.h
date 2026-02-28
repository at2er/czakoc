/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_FN_H
#define CZAKOC_FN_H
#include <mcb/func.h>
#include <stdbool.h>
#include "stmt.h"

struct zako_ident;
struct zako_type;
struct zako_value;

struct zako_fn_call {
	struct zako_ident *fn;
	int argc;
	struct zako_value **args;
};

struct zako_fn_declaration {
	struct zako_ident *ident;
	bool public;
};

struct zako_fn_definition {
	struct zako_fn_declaration *declaration;
	struct zako_stmt **stmts;
	size_t stmts_count;
};

struct zako_fn_type {
	int argc;
	struct zako_ident **args;
	struct zako_type *type;

	struct mcb_func *mcb_fn;
};

void destroy_fn_type(struct zako_fn_type *self);
void free_fn_call(struct zako_fn_call *self);
void free_fn_declaration(struct zako_fn_declaration *self);
void free_fn_definition(struct zako_fn_definition *self);
void print_fn_call(struct zako_fn_call *self, Jim *jim);
void print_fn_definition(struct zako_fn_definition *self, Jim *jim);

#endif
