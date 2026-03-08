/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <mcb/inst/branch.h>
#include <mcb/inst/cmp.h>
#include <mcb/label.h>
#include <mcb/value.h>
#include "compiler.h"
#include "expr.h"
#include "if.h"
#include "stmt.h"
#include "../if.h"
#include "../panic.h"

int
compile_if_stmt(
		struct zako_if_stmt *stmt,
		struct compiler_context *ctx)
{
	struct mcb_value *cmp_result;
	struct mcb_label *on_false, *on_true;

	cmp_result = compile_cmp_expr(stmt->cond, ctx);
	on_false = mcb_define_label(NULL, ctx->fn);
	on_true = mcb_define_label(NULL, ctx->fn);

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
