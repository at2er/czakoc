/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_VALUE_H
#define CZAKOC_VALUE_H
#include <stdint.h>
#include "type.h"

struct zako_expr;
struct zako_fn_call;
struct zako_ident;

struct zako_value {
	enum VALUE_KIND {
		EXPR_VALUE,
		FN_CALL_VALUE,
		IDENT_VALUE,
		INT_LITERAL
	} kind;
	union {
		struct zako_expr *expr;
		struct zako_fn_call *fn_call;
		struct zako_ident *ident;
		int64_t  i;
		uint64_t u;
	} data;

	struct zako_type *type;
};

void free_value(struct zako_value *self);
void print_value(struct zako_value *self, Jim *jim);

#endif
