/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include <stdbool.h>
#include "semantics.h"
#include "../ident.h"
#include "../parser/parser.h"
#include "../type.h"

static int analyse_binary_expr(
		struct zako_expr *expr,
		struct zako_type *expect_type);
static int analyse_fn_call(
		struct zako_fn_call *call,
		struct zako_type *expect_type);
static int analyse_ident(
		struct zako_ident *ident,
		struct zako_type *expect_type);
static int analyse_value(
		struct zako_value *value,
		struct zako_type *expect_type);
static int analyse_primary_expr(
		struct zako_expr *expr,
		struct zako_type *expect_type);
static int compare_builtin_type(
		enum ZAKO_BUILTIN_TYPE src,
		enum ZAKO_BUILTIN_TYPE expect);

static const char *analysis_result_str[] = {
	[SUCCESS] = "analysis succeed",
	[UNKOWN_ANALYSIS_ERR] = "unkown analysis error",
	[TYPE_COMPARE_EXPECT_SIGNED] =
		"type compare failed: expect signed integer",
	[TYPE_COMPARE_EXPECT_UNSIGNED] =
		"type compare failed: expect unsigned integer",
	[TYPE_COMPARE_IMPLICIT_CAST] =
		"type compare failed: implicit cast"
};

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
analyse_fn_call(struct zako_fn_call *call, struct zako_type *expect_type)
{
	struct zako_fn_type *fn_type;
	int ret;
	assert(call && expect_type);
	fn_type = &call->fn->type->inner.fn;
	for (int i = 0; i < call->argc; i++) {
		ret = analyse_value(call->args[i], fn_type->args[i]->type);
		if (ret)
			return ret;
	}
	return compare_builtin_type(call->fn->type->inner
			.fn.type->builtin,
			expect_type->builtin);
}

int
analyse_ident(struct zako_ident *ident, struct zako_type *expect_type)
{
	assert(ident && expect_type);
	return compare_builtin_type(ident->type->builtin, expect_type->builtin);
}

int
analyse_value(
		struct zako_value *value,
		struct zako_type *expect_type)
{
	int ret;
	assert(value && expect_type);
	switch (value->kind) {
	case EXPR_VALUE:
		ret = analyse_expr(value->data.expr, expect_type);
		if (ret)
			return ret;
		break;
	case FN_CALL_VALUE:
		ret = analyse_fn_call(value->data.fn_call, expect_type);
		if (ret)
			return ret;
		break;
	case IDENT_VALUE:
		ret = analyse_ident(value->data.ident, expect_type);
		if (ret)
			return ret;
		break;
	case INT_LITERAL:
		if (!IS_NUMBER(expect_type->builtin))
			return TYPE_COMPARE_IMPLICIT_CAST;
		break;
	}
	value->type = expect_type;
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

int
compare_builtin_type(enum ZAKO_BUILTIN_TYPE src, enum ZAKO_BUILTIN_TYPE expect)
{
	if (expect == src)
		return SUCCESS;
	if (IS_SIGNED_NUMBER(expect)) {
		if (!IS_SIGNED_NUMBER(src))
			return TYPE_COMPARE_EXPECT_SIGNED;
		if (src <= expect)
			return SUCCESS;
		return TYPE_COMPARE_IMPLICIT_CAST;
	}
	if (IS_UNSIGNED_NUMBER(expect)) {
		if (!IS_UNSIGNED_NUMBER(src))
			return TYPE_COMPARE_EXPECT_UNSIGNED;
		if (src <= expect)
			return SUCCESS;
		return TYPE_COMPARE_IMPLICIT_CAST;
	}
	return UNKOWN_ANALYSIS_ERR;
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
	case PRIMARY_EXPR:
		ret = analyse_primary_expr(expr, expect_type);
		break;
	}
	if (ret)
		return ret;
	expr->type = expect_type;
	return SUCCESS;
}

const char *
cstr_analysis_result(enum ANALYSIS_RESULT result)
{
	return analysis_result_str[result];
}
