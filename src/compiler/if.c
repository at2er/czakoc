/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <mcb/inst/branch.h>
#include <mcb/inst/cmp.h>
#include <mcb/label.h>
#include <mcb/type.h>
#include <mcb/value.h>
#include "compiler.h"
#include "if.h"
#include "stmt.h"
#include "value.h"
#include "../if.h"
#include "../panic.h"

static struct mcb_value *compile_if_cond(
		struct zako_expr *expr,
		struct compiler_context *ctx);
static enum MCB_CMP_OPERATOR mcb_cmp_op_from_zako(enum ZAKO_SYMBOL sym);

struct mcb_value *
compile_if_cond(
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

enum MCB_CMP_OPERATOR
mcb_cmp_op_from_zako(enum ZAKO_SYMBOL sym)
{
	switch (sym) {
	case SYM_INFIX_LESS_EQUAL:
		return MCB_LE;
	default:
		break;
	}
	panic("mcb_cmp_op_from_zako()");
	return -1;
}

int
compile_if_stmt(
		struct zako_if_stmt *stmt,
		struct compiler_context *ctx)
{
	struct mcb_value *cmp_result;
	struct mcb_label *on_false, *on_true;
	cmp_result = compile_if_cond(stmt->cond, ctx);
	on_false = mcb_define_label("on_false");
	on_true = mcb_define_label("on_true");
	if (mcb_inst_branch(cmp_result, on_true, on_false, ctx->fn))
		panic("mcb_inst_branch()");
	if (mcb_append_label(on_true, ctx->fn))
		panic("mcb_append_label()");
	for (size_t i = 0; i < stmt->stmts_count; i++) {
		if (compile_stmt(stmt->stmts[i], ctx))
			return 1;
	}
	if (mcb_append_label(on_false, ctx->fn))
		panic("mcb_append_label()");
	return 0;
}
