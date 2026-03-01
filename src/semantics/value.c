/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include "expr.h"
#include "fn.h"
#include "ident.h"
#include "semantics.h"
#include "value.h"
#include "../type.h"
#include "../value.h"

static int analyse_arr_elem_value(
		struct zako_arr_elem_value *elem,
		struct zako_type *expect_type);
static int analyse_arr_init_value(
		struct zako_elem_init_value *elem_init,
		struct zako_type *expect_type);
static int analyse_elem_init_value(
		struct zako_elem_init_value *elem_init,
		struct zako_type *expect_type);
static int analyse_string_literal(
		struct str *str,
		struct zako_type *expect_type);

int
analyse_arr_elem_value(
		struct zako_arr_elem_value *elem,
		struct zako_type *expect_type)
{
	struct zako_arr_type *arr_type;
	struct zako_type *elem_type;
	assert(elem && expect_type);
	arr_type = &elem->arr->type->inner.arr;
	elem_type = arr_type->elem_type;
	if (arr_type->size != 0 && elem->idx >= arr_type->size)
		return ACCESS_OUT_OF_ARR;
	return compare_builtin_type(elem_type->builtin, expect_type->builtin);
}

int
analyse_arr_init_value(
		struct zako_elem_init_value *elem_init,
		struct zako_type *expect_type)
{
	struct zako_arr_type *arr_type;
	int ret;
	assert(elem_init && expect_type);
	assert(expect_type->builtin == ARR_TYPE);
	arr_type = &expect_type->inner.arr;
	for (size_t i = 0; i < elem_init->elems_count; i++) {
		ret = analyse_value(elem_init->elems[i], arr_type->elem_type);
		if (ret)
			return ret;
	}
	return 0;
}

int
analyse_elem_init_value(
		struct zako_elem_init_value *elem_init,
		struct zako_type *expect_type)
{
	assert(elem_init && expect_type);
	switch (expect_type->builtin) {
	case ARR_TYPE:
		return analyse_arr_init_value(elem_init, expect_type);
	default:
		break;
	}
	return CANNOT_INIT_VALUE_BY_ELEM_INIT;
}

int
analyse_string_literal(
		struct str *str,
		struct zako_type *expect_type)
{
	enum ZAKO_BUILTIN_TYPE arr_elem_builtin;
	assert(str && expect_type);
	if (expect_type->builtin != ARR_TYPE)
		return TYPE_COMPARE_IMPLICIT_CAST;
	arr_elem_builtin = expect_type->inner.arr.elem_type->builtin;
	if (arr_elem_builtin != U8_TYPE && arr_elem_builtin != I8_TYPE)
		return TYPE_COMPARE_IMPLICIT_CAST;
	return 0;
}

enum ANALYSIS_RESULT
analyse_value(struct zako_value *value, struct zako_type *expect_type)
{
	int ret;
	assert(value && expect_type);
	switch (value->kind) {
	case ARR_ELEM_VALUE:
		ret = analyse_arr_elem_value(
				&value->data.arr_elem,
				expect_type);
		if (ret)
			return ret;
		break;
	case ELEM_INIT_VALUE:
		ret = analyse_elem_init_value(
				&value->data.elem_init,
				expect_type);
		if (ret)
			return ret;
		break;
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
	case STRING_LITERAL:
		ret = analyse_string_literal(&value->data.str, expect_type);
		if (ret)
			return ret;
		break;
	}
	value->type = expect_type;
	return 0;
}
