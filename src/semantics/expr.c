/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include "expr.h"
#include "semantics.h"
#include "value.h"
#include "../ealloc.h"
#include "../ident.h"
#include "../panic.h"
#include "../value.h"

static int analyse_binary_expr(
		struct zako_expr *expr,
		struct zako_type *expect_type);
static int analyse_binary_expr_with_cast(
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
	int ret_lhs, ret_rhs;
	assert(expr && expect_type);
	binary = &expr->inner.binary;

	expr->type = expect_type;
	if (binary->op == SYM_INFIX_AS)
		return analyse_binary_expr_with_cast(expr, expect_type);

	ret_lhs = analyse_value(binary->lhs, expect_type);
	ret_rhs = analyse_value(binary->rhs, expect_type);

	if (ret_lhs)
		return ret_lhs;
	if (ret_rhs)
		return ret_rhs;
	return 0;
}

int
analyse_binary_expr_with_cast(
		struct zako_expr *expr,
		struct zako_type *expect_type)
{
	struct zako_binary_expr *binary;
	int ret;
	assert(expr && expect_type);
	binary = &expr->inner.binary;
	assert(binary->op == SYM_INFIX_AS);

	assert(binary->rhs->kind == TYPE_LITERAL);
	ret = compare_builtin_type(
			binary->rhs->data.type->builtin,
			expect_type->builtin);
	if (ret)
		return ret;
	binary->rhs->type = expect_type;

	analyse_value(binary->lhs, expect_type);
	return analyse_value(binary->rhs, expect_type);
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
	struct zako_ident *ident = NULL;
	int ret;
	struct zako_type *type = NULL;
	assert(expr);

	switch (expr->inner.binary.lhs->kind) {
	case ARR_ELEM_VALUE:
		ident = expr->inner.binary.lhs->data.arr_elem.arr;
		type = ident->type->inner.arr.elem_type;
		break;
	case IDENT_VALUE:
		ident = expr->inner.binary.lhs->data.ident;
		type = ident->type;
		break;
	default:
		break;
	}

	if (!ident->mutable)
		return IDENT_CANNOT_BE_ASSIGNED;

	ret = analyse_expr(expr, type);
	if (ret)
		return ret;
	return SUCCESS;
}
