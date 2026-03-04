/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include <mcb/inst.h>
#include <mcb/inst/add.h>
#include <mcb/inst/cmp.h>
#include <mcb/inst/div.h>
#include <mcb/inst/mul.h>
#include <mcb/inst/store.h>
#include <mcb/inst/sub.h>
#include <mcb/type.h>
#include <mcb/value.h>
#include "compiler.h"
#include "expr.h"
#include "fn.h"
#include "utils.h"
#include "value.h"
#define UTILSH_CONTAINER_OF_STRIP
#include "../container_of.h"
#include "../expr.h"
#include "../ident.h"
#include "../panic.h"
#include "../type.h"

static int compile_assign_expr(
		struct zako_binary_expr *binary,
		struct compiler_context *ctx);
static struct mcb_value *compile_binary_expr(
		struct zako_binary_expr *binary,
		struct compiler_context *ctx);
static struct mcb_value *compile_primary_expr(
		struct zako_value *primary,
		struct compiler_context *ctx);
static enum MCB_CMP_OPERATOR mcb_cmp_op_from_zako(enum ZAKO_SYMBOL sym);

int
compile_assign_expr(
		struct zako_binary_expr *binary,
		struct compiler_context *ctx)
{
	struct mcb_value *container, *val;
	assert(binary && ctx);
	container = binary->lhs->data.ident->value;
	val = compile_value(binary->rhs, ctx);
	assert(container && val);
	if (mcb_inst_store_value(container, val, ctx->fn))
		panic("mcb_inst_store_value()");
	return 0;
}

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

enum MCB_CMP_OPERATOR
mcb_cmp_op_from_zako(enum ZAKO_SYMBOL sym)
{
	switch (sym) {
	case SYM_INFIX_EQUAL:
		return MCB_EQ;
	case SYM_INFIX_GREATER:
		return MCB_GT;
	case SYM_INFIX_GREATER_EQUAL:
		return MCB_GE;
	case SYM_INFIX_LESS:
		return MCB_LT;
	case SYM_INFIX_LESS_EQUAL:
		return MCB_LE;
	case SYM_INFIX_NOT_EQUAL:
		return MCB_NE;
	default:
		break;
	}
	panic("mcb_cmp_op_from_zako()");
	return -1;
}

struct mcb_value *
compile_cmp_expr(
		struct zako_expr *expr,
		struct compiler_context *ctx)
{
	enum MCB_CMP_OPERATOR op;
	struct mcb_value *result, *lhs, *rhs;
	assert(expr && ctx);
	op = mcb_cmp_op_from_zako(expr->inner.cmp.op);
	lhs = compile_value(expr->inner.cmp.lhs, ctx);
	if (!lhs)
		panic("compile_value()");
	rhs = compile_value(expr->inner.cmp.rhs, ctx);
	if (!rhs)
		panic("compile_value()");
	result = mcb_define_value(
			"cmp_result",
			mcb_get_type_from_builtin(MCB_CMP_RESULT),
			ctx->fn);
	if (!result)
		panic("mcb_define_value()");
	if (mcb_inst_cmp(result, lhs, op, rhs, ctx->fn))
		panic("mcb_inst_cmp()");
	return result;
}

struct mcb_value *
compile_expr(struct zako_expr *expr, struct compiler_context *ctx)
{
	assert(expr && ctx);
	switch (expr->kind) {
	case BINARY_EXPR:
		return compile_binary_expr(&expr->inner.binary, ctx);
	case CMP_EXPR:
		panic("expr->kind == CMP_EXPR");
		break;
	case PRIMARY_EXPR:
		return compile_primary_expr(expr->inner.primary, ctx);
	}
	return NULL;
}

int
compile_expr_stmt(struct zako_expr *expr, struct compiler_context *ctx)
{
	assert(expr && ctx);
	if (expr->kind == PRIMARY_EXPR) {
		compile_fn_call(expr->inner.primary->data.fn_call, ctx);
		mcb_force_gen_inst(ctx->fn);
		return 0;
	}

	assert(expr->kind == BINARY_EXPR);
	switch (expr->inner.binary.op) {
	case SYM_ASSIGN:
		return compile_assign_expr(&expr->inner.binary, ctx);
	default:
		break;
	}
	panic("expr->inner.binary.op");
	return 0;
}
