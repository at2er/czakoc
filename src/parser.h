/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_PARSER_H
#define CZAKOC_PARSER_H
#include <stdint.h>
#include "fn.h"
#include "module.h"
#include "type.h"

/*
enum EXPR_OPERATOR {
};
*/

struct zako_declaration {
	enum DECLARATION_KIND {
		FN_DECLARATION
	} kind;
	union {
		struct zako_fn_declaration *fn_declaration;
	} inner;
};

struct zako_definition {
	enum DEFINITION_KIND {
		FN_DEFINITION
	} kind;
	union {
		struct zako_fn_definition *fn_defintion;
	} inner;
};

struct zako_expr {
	// enum EXPR_OPERATOR operator;
	int argc;
	struct zako_literal **args;
};

struct zako_fn_declaration {
	struct zako_ident *ident;
};

struct zako_fn_definition {
	struct zako_fn_declaration *declaration;
	struct zako_stmt **stmts;
	size_t stmts_count;
};

struct zako_literal {
	enum LITERAL_KIND {
		INT_LITERAL
	} kind;
	union {
		int64_t  i;
		uint64_t u;
	} data;
};

struct zako_return_stmt {
	struct zako_expr *expr;
};

struct zako_stmt {
	enum STMT_KIND {
		RETURN_STMT
	} kind;
	union {
		struct zako_return_stmt *return_stmt;
	} inner;
};

struct zako_toplevel_stmt {
	enum TOPLEVEL_STMT_KIND {
		DECLARATION_STMT,
		DEFINITION_STMT
	} kind;
	union {
		struct zako_declaration declaration;
		struct zako_definition definition;
	} inner;
};

void free_zako_expr(struct zako_expr *self);
void free_zako_fn_declaration(struct zako_fn_declaration *self);
void free_zako_fn_definition(struct zako_fn_definition *self);
void free_zako_literal(struct zako_literal *self);
void free_zako_return_stmt(struct zako_return_stmt *self);
void free_zako_stmt(struct zako_stmt *self);
void free_zako_toplevel_stmt(struct zako_toplevel_stmt *self);
struct zako_module *parse_file(const char *path);

#endif
