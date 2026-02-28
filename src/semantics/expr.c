/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include "expr.h"
#include "semantics.h"
#include "value.h"
#include "../ealloc.h"
#include "../ident.h"
#include "../panic.h"

static int analyse_binary_expr(
		struct zako_expr *expr,
		struct zako_type *expect_type);
static int analyse_cmp_expr_value(
		struct zako_value *val,
		struct zako_value *another);
static int analyse_primary_expr(
		struct zako_expr *expr,
		struct zako_type *expect_type);

int
analyse_binary_expr(
		struct zako_expr *expr,
		struct zako_type *expect_type)
{
	struct zako_binary_expr *binary;
	int ret;
	assert(expr && expect_type);
	binary = &expr->inner.binary;
	ret = analyse_value(binary->lhs, expect_type);
	if (ret)
		return ret;
	ret = analyse_value(binary->rhs, expect_type);
	if (ret)
		return ret;
	return 0;
}

int
analyse_cmp_expr_value(struct zako_value *val, struct zako_value *another)
{
	enum ZAKO_BUILTIN_TYPE builtin;
	if (val->type)
		return 0;
	if (val->kind == INT_LITERAL) {
		builtin = get_builtin_type_from_int_literal(val->data.i);
		if (another->type &&
				IS_NUMBER(another->type->builtin) &&
				another->type->builtin >= builtin)
			builtin = another->type->builtin;
		val->type = ecalloc(1, sizeof(*val->type));
		val->type->builtin = builtin;
	} else if (val->kind == IDENT_VALUE) {
		val->type = val->data.ident->type;
	}
	return 0;
}

int
analyse_primary_expr(
		struct zako_expr *expr,
		struct zako_type *expect_type)
{
	assert(expr && expect_type);
	return analyse_value(expr->inner.primary, expect_type);
}

enum ANALYSIS_RESULT
analyse_cmp_expr(struct zako_expr *expr)
{
	int ret;
	assert(expr);
	assert(expr->kind == CMP_EXPR);
	ret = analyse_cmp_expr_value(expr->inner.cmp.lhs, expr->inner.cmp.rhs);
	if (ret)
		return ret;
	ret = analyse_cmp_expr_value(expr->inner.cmp.rhs, expr->inner.cmp.lhs);
	if (ret)
		return ret;
	return 0;
}

enum ANALYSIS_RESULT
analyse_expr(struct zako_expr *expr, struct zako_type *expect_type)
{
	int ret = 0;
	assert(expr && expect_type);
	switch (expr->kind) {
	case BINARY_EXPR:
		ret = analyse_binary_expr(expr, expect_type);
		break;
	case CMP_EXPR:
		panic("please use analyse_cmp_expr");
		break;
	case PRIMARY_EXPR:
		ret = analyse_primary_expr(expr, expect_type);
		break;
	}
	if (ret)
		return ret;
	expr->type = expect_type;
	return SUCCESS;
}

enum ANALYSIS_RESULT
analyse_expr_stmt(struct zako_expr *expr)
{
	struct zako_ident *ident;
	int ret;
	assert(expr);
	ident = expr->inner.binary.lhs->data.ident;
	if (!ident->mutable)
		return IDENT_CANNOT_BE_ASSIGNED;
	ret = analyse_expr(expr, ident->type);
	if (ret)
		return ret;
	return SUCCESS;
}
