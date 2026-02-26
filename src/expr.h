/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_EXPR_H
#define CZAKOC_EXPR_H
#include "jim2.h"
#include "lexer.h"
#include "type.h"
#include "value.h"

struct zako_binary_expr {
	enum ZAKO_SYMBOL op;
	struct zako_value *lhs, *rhs;
};

struct zako_compare_expr {
	enum ZAKO_SYMBOL op;
	struct zako_value *lhs, *rhs;
};

struct zako_expr {
	enum EXPR_KIND {
		BINARY_EXPR,
		CMP_EXPR,
		PRIMARY_EXPR
	} kind;
	union {
		struct zako_binary_expr binary;
		struct zako_compare_expr cmp;
		struct zako_value *primary;
	} inner;

	struct zako_type *type;
};

void free_zako_expr(struct zako_expr *self);
void print_expr(struct zako_expr *self, Jim *jim);

#endif
