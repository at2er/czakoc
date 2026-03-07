/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include <mcb/inst/alloc_array.h>
#include <mcb/inst/alloc_var.h>
#include <mcb/inst/store.h>
#include <mcb/value.h>
#include "compiler.h"
#include "expr.h"
#include "let.h"
#include "type.h"
#include "value.h"
#include "../ealloc.h"
#include "../panic.h"
#include "../let.h"
#include "../type.h"

static struct mcb_value *compile_arr_let_stmt(
		struct zako_let_stmt *stmt,
		struct compiler_context *ctx);
static struct mcb_value *compile_numbers_let_stmt(
		struct zako_let_stmt *stmt,
		struct compiler_context *ctx);

struct mcb_value *
compile_arr_let_stmt(
		struct zako_let_stmt *stmt,
		struct compiler_context *ctx)
{
	struct zako_arr_type *arr_type;
	struct mcb_value *container, *elem, *value;
	struct zako_elem_init_value *elem_init;
	struct mcb_type *mcb_arr_type;

	assert(stmt && ctx);
	assert(stmt->expr->kind == PRIMARY_EXPR);
	assert(stmt->ident->type->builtin == ARR_TYPE);

	arr_type = &stmt->ident->type->inner.arr;
	elem_init = &stmt->expr->inner.primary->data.elem_init;

	mcb_arr_type = ecalloc(1, sizeof(*mcb_arr_type));
	mcb_arr_type->builtin = MCB_ARRAY;
	mcb_arr_type->inner = (struct mcb_type*)
		mcb_type_from_zako(arr_type->elem_type);

	container = mcb_define_value(
			stmt->ident->name,
			mcb_arr_type,
			ctx->fn);
	if (mcb_inst_alloc_array(container, arr_type->size, ctx->fn))
		panic("mcb_inst_alloc_array()");

	for (size_t i = 0; i < arr_type->size; i++) {
		elem = container->inner.array.elems[i];
		value = compile_value(elem_init->elems[i], ctx);
		if (mcb_inst_store_value(elem, value, ctx->fn))
			panic("mcb_inst_store_value()");
	}

	return container;
}

struct mcb_value *
compile_numbers_let_stmt(
		struct zako_let_stmt *stmt,
		struct compiler_context *ctx)
{
	struct mcb_value *container, *value;
	assert(stmt && ctx);
	container = mcb_define_value(
			stmt->ident->name,
			mcb_type_from_zako(stmt->ident->type),
			ctx->fn);
	if (mcb_inst_alloc_var(container, ctx->fn))
		panic("mcb_inst_alloc_var()");
	if (!container)
		panic("mcb_define_value()");
	value = compile_expr(stmt->expr, ctx);
	if (mcb_inst_store_value(container, value, ctx->fn))
		panic("mcb_inst_store_value()");
	return container;
}

int
compile_let_stmt(
		struct zako_let_stmt *stmt,
		struct compiler_context *ctx)
{
	struct mcb_value *container;
	assert(stmt && ctx);
	switch (stmt->ident->type->builtin) {
	CASE_NUMBERS:
		container = compile_numbers_let_stmt(stmt, ctx);
		break;
	case ARR_TYPE:
		container = compile_arr_let_stmt(stmt, ctx);
		break;
	default:
		panic("stmt->ident->type->builtin");
		return 1;
	}
	if (!container)
		panic("!container");
	stmt->ident->value = container;
	return 0;
}
