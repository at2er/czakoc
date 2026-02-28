/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <mcb/array.h>
#include <mcb/inst/store.h>
#include <mcb/value.h>
#include "compiler.h"
#include "expr.h"
#include "fn.h"
#include "utils.h"
#include "value.h"
#include "../ident.h"
#include "../panic.h"
#include "../value.h"

static struct mcb_value *compile_arr_elem_value(
		struct zako_arr_elem_value *value,
		struct compiler_context *ctx);

struct mcb_value *
compile_arr_elem_value(
		struct zako_arr_elem_value *value,
		struct compiler_context *ctx)
{
	struct mcb_value *result;
	assert(value && ctx);
	result = mcb_get_value_from_array(
			value->arr->value,
			value->idx,
			ctx->fn);
	if (!result)
		panic("mcb_get_value_from_array()");
	return result;
}

struct mcb_value *
compile_value(struct zako_value *value, struct compiler_context *ctx)
{
	struct mcb_value *mcb_val;
	assert(value && ctx);
	switch (value->kind) {
	case ARR_ELEM_VALUE:
		return compile_arr_elem_value(&value->data.arr_elem, ctx);
	case ELEM_INIT_VALUE:
		panic("value->kind == ELEM_INIT_VALUE");
		break;
	case EXPR_VALUE:
		return compile_expr(value->data.expr, ctx);
	case FN_CALL_VALUE:
		return compile_fn_call(value->data.fn_call, ctx);
	case IDENT_VALUE:
		return value->data.ident->value;
	case INT_LITERAL:
		mcb_val = mcb_define_value(
				"_",
				mcb_type_from_zako(value->type),
				ctx->fn);
		if (!mcb_val)
			panic("mcb_define_value()");
		if (mcb_inst_store_int(mcb_val, value->data.i, ctx->fn))
			panic("mcb_inst_store_int()");
		break;
	}
	return mcb_val;
}
