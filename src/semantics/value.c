/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include "expr.h"
#include "fn.h"
#include "ident.h"
#include "semantics.h"
#include "value.h"
#include "../type.h"
#include "../value.h"

static int analyse_elem_init_value(
		struct zako_elem_init_value *elem_init,
		struct zako_type *expect_type);

int
analyse_elem_init_value(
		struct zako_elem_init_value *elem_init,
		struct zako_type *expect_type)
{
	assert(elem_init && expect_type);
	return 0;
}

enum ANALYSIS_RESULT
analyse_value(struct zako_value *value, struct zako_type *expect_type)
{
	int ret;
	assert(value && expect_type);
	switch (value->kind) {
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
	}
	value->type = expect_type;
	return 0;
}
