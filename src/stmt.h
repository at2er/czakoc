/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_STMT_H
#define CZAKOC_STMT_H
#include "jim2.h"

struct zako_expr;
struct zako_if_stmt;
struct zako_let_stmt;
struct zako_return_stmt;

struct zako_stmt {
	enum STMT_KIND {
		EXPR_STMT,
		IF_STMT,
		LET_STMT,
		RETURN_STMT
	} kind;
	union {
		struct zako_expr *expr_stmt;
		struct zako_if_stmt *if_stmt;
		struct zako_let_stmt *let_stmt;
		struct zako_return_stmt *return_stmt;
	} inner;
};

struct zako_toplevel_stmt {
	enum TOPLEVEL_STMT_KIND {
		FN_DECLARATION,
		FN_DEFINITION
	} kind;
	union {
		struct zako_fn_declaration *fn_declaration;
		struct zako_fn_definition *fn_definition;
	} inner;
};

void free_stmt(struct zako_stmt *self);
void free_toplevel_stmt(struct zako_toplevel_stmt *self);
void print_stmt(struct zako_stmt *self, Jim *jim);
void print_toplevel_stmt(struct zako_toplevel_stmt *self, Jim *jim);

#endif
