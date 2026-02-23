/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include <stdbool.h>
#include "parser.h"
#include "semantics.h"
#include "type.h"

static int analyse_binary_expr(
		struct zako_expr *expr,
		struct zako_type *expect_type);
static int analyse_literal(
		struct zako_literal *literal,
		struct zako_type *expect_type);
static int analyse_primary_expr(
		struct zako_expr *expr,
		struct zako_type *expect_type);

int
analyse_binary_expr(
		struct zako_expr *expr,
		struct zako_type *expect_type)
{
	struct zako_binary_expr *binary;
	assert(expr && expect_type);
	binary = &expr->inner.binary;
	if (analyse_literal(binary->lhs, expect_type))
		return 1;
	if (analyse_literal(binary->rhs, expect_type))
		return 1;
	return 0;
}

int
analyse_literal(
		struct zako_literal *literal,
		struct zako_type *expect_type)
{
	assert(literal && expect_type);
	switch (literal->kind) {
	case EXPR_LITERAL:
		if (analyse_expr(literal->data.expr, expect_type))
			return 1;
		break;
	case INT_LITERAL:
		if (!IS_NUMBER(expect_type->builtin))
			return 1;
		break;
	}
	literal->type = expect_type;
	return 0;
}

int
analyse_primary_expr(
		struct zako_expr *expr,
		struct zako_type *expect_type)
{
	assert(expr && expect_type);
	return analyse_literal(expr->inner.primary, expect_type);
}

int
analyse_expr(struct zako_expr *expr, struct zako_type *expect_type)
{
	int ret = 0;
	assert(expr && expect_type);
	switch (expr->kind) {
	case BINARY_EXPR:
		ret = analyse_binary_expr(expr, expect_type);
		break;
	case PRIMARY_EXPR:
		ret = analyse_primary_expr(expr, expect_type);
		break;
	}
	if (ret)
		return 1;
	expr->type = expect_type;
	return 0;
}
