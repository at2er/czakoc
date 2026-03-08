/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <mcb/inst/address_of.h>
#include <mcb/inst/element_of.h>
#include <mcb/inst/store.h>
#include <mcb/value.h>
#include "compiler.h"
#include "expr.h"
#include "fn.h"
#include "type.h"
#include "value.h"
#include "../ident.h"
#include "../panic.h"
#include "../value.h"

static struct mcb_value *compile_arr_value(
		struct zako_ident *ident,
		struct compiler_context *ctx);
static struct mcb_value *compile_arr_elem_value(
		struct zako_arr_elem_value *value,
		struct compiler_context *ctx);

struct mcb_value *
compile_arr_value(
		struct zako_ident *ident,
		struct compiler_context *ctx)
{
	struct mcb_value *result;
	const struct mcb_type *type;
	assert(ident && ctx);
	type = mcb_type_from_zako(ident->type);
	result = mcb_define_value(NULL, type, ctx->fn);
	if (mcb_inst_address_of(result, ident->value, ctx->fn))
		panic("mcb_inst_address_of");
	return result;
}

struct mcb_value *
compile_arr_elem_value(
		struct zako_arr_elem_value *value,
		struct compiler_context *ctx)
{
	struct mcb_value *result, *idx;
	assert(value && ctx);
	result = mcb_define_value(NULL,
			value->arr->value->type->inner,
			ctx->fn);
	idx = compile_value(value->idx, ctx);
	if (mcb_inst_element_of(result, value->arr->value, idx, ctx->fn))
		panic("mcb_inst_element_of()");
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
		if (value->data.ident->type->builtin != ARR_TYPE)
			return value->data.ident->value;
		mcb_val = compile_arr_value(value->data.ident, ctx);
		break;
	case INT_LITERAL:
		mcb_val = mcb_define_value(NULL,
				mcb_type_from_zako(value->type),
				ctx->fn);
		if (!mcb_val)
			panic("mcb_define_value()");
		if (mcb_inst_store_int(mcb_val, value->data.i, ctx->fn))
			panic("mcb_inst_store_int()");
		break;
	case STRING_LITERAL:
		mcb_val = mcb_define_value(NULL,
				mcb_type_from_zako(value->type),
				ctx->fn);
		if (!mcb_val)
			panic("mcb_define_value()");
		if (mcb_inst_store_string(
					mcb_val,
					value->data.str.s,
					value->data.str.len,
					ctx->fn))
			panic("mcb_inst_store_int()");
		break;
	case TYPE_LITERAL:
		panic("TYPE_LITERAL");
		break;
	}
	return mcb_val;
}
