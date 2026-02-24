/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_PARSER_H
#define CZAKOC_PARSER_H
#include <stddef.h>
#include <stdint.h>
#include "fn.h"
#include "jim2.h"
#include "module.h"
#include "type.h"

enum ZAKO_SYMBOL {
	SYM_PAREN_L,
	SYM_PAREN_R,
	SYM_ASSIGN,
	SYM_COMMA,
	SYM_INFIX_ADD,
	SYM_INFIX_DIV,
	SYM_INFIX_MUL,
	SYM_INFIX_SUB
};

struct zako_binary_expr {
	enum ZAKO_SYMBOL op;
	struct zako_literal *lhs, *rhs;
};

struct zako_expr {
	enum EXPR_KIND {
		BINARY_EXPR,
		PRIMARY_EXPR
	} kind;
	union {
		struct zako_binary_expr binary;
		struct zako_literal *primary;
	} inner;

	struct zako_type *type;
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
		EXPR_LITERAL,
		INT_LITERAL
	} kind;
	union {
		struct zako_expr *expr;
		int64_t  i;
		uint64_t u;
	} data;

	struct zako_type *type;
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
		FN_DECLARATION,
		FN_DEFINITION
	} kind;
	union {
		struct zako_fn_declaration *fn_declaration;
		struct zako_fn_definition *fn_definition;
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

void print_expr(struct zako_expr *self, Jim *jim);
void print_fn_definition(struct zako_fn_definition *self, Jim *jim);
void print_ident(struct zako_ident *self, Jim *jim);
void print_literal(struct zako_literal *self, Jim *jim);
void print_stmt(struct zako_stmt *self, Jim *jim);
void print_toplevel_stmt(struct zako_toplevel_stmt *self, Jim *jim);
void print_type(struct zako_type *self, Jim *jim);

#endif
