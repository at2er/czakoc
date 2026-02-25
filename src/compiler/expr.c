/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include <mcb/value.h>
#include <mcb/inst/add.h>
#include <mcb/inst/div.h>
#include <mcb/inst/mul.h>
#include <mcb/inst/sub.h>
#include "compiler.h"
#include "expr.h"
#include "utils.h"
#include "value.h"
#define UTILSH_CONTAINER_OF_STRIP
#include "../container_of.h"
#include "../expr.h"
#include "../panic.h"
#include "../type.h"

static struct mcb_value *compile_binary_expr(
		struct zako_binary_expr *binary,
		struct compiler_context *ctx);
static struct mcb_value *compile_primary_expr(
		struct zako_value *primary,
		struct compiler_context *ctx);

struct mcb_value *
compile_primary_expr(
		struct zako_value *primary,
		struct compiler_context *ctx)
{
	assert(primary && ctx);
	return compile_value(primary, ctx);
}

struct mcb_value *
compile_binary_expr(
		struct zako_binary_expr *binary,
		struct compiler_context *ctx)
{
	struct zako_expr *expr;
	struct mcb_value *result, *rem, *lhs, *rhs;
	assert(binary && ctx);
	expr = container_of(binary, struct zako_expr, inner.binary);
	result = mcb_define_value(
			"binary_expr_result",
			mcb_type_from_zako(expr->type),
			ctx->fn);
	lhs = compile_value(binary->lhs, ctx);
	rhs = compile_value(binary->rhs, ctx);
	switch (binary->op) {
	case SYM_INFIX_ADD:
		if (mcb_inst_add(result, lhs, rhs, ctx->fn))
			panic("mcb_inst_add()");
		break;
	case SYM_INFIX_DIV:
		rem = mcb_define_value("_rem_", result->type, ctx->fn);
		if (mcb_inst_div(result, rem, lhs, rhs, ctx->fn))
			panic("mcb_inst_div()");
		break;
	case SYM_INFIX_MUL:
		if (mcb_inst_mul(result, lhs, rhs, ctx->fn))
			panic("mcb_inst_mul()");
		break;
	case SYM_INFIX_SUB:
		if (mcb_inst_sub(result, lhs, rhs, ctx->fn))
			panic("mcb_inst_sub()");
		break;
	default:
		panic("binary->op not infix");
		break;
	}
	return result;
}

struct mcb_value *
compile_expr(struct zako_expr *expr, struct compiler_context *ctx)
{
	assert(expr && ctx);
	switch (expr->kind) {
	case BINARY_EXPR:
		return compile_binary_expr(&expr->inner.binary, ctx);
	case PRIMARY_EXPR:
		return compile_primary_expr(expr->inner.primary, ctx);
	}
	return NULL;
}
