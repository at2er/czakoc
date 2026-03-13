/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <mcb/inst/branch.h>
#include <mcb/inst/define_label.h>
#include <mcb/inst/jmp.h>
#include <mcb/label.h>
#include <mcb/value.h>
#include "compiler.h"
#include "expr.h"
#include "while.h"
#include "stmt.h"
#include "../panic.h"
#include "../while.h"

int
compile_while_stmt(
		struct zako_while_stmt *stmt,
		struct compiler_context *ctx)
{
	struct mcb_value *cmp_result;
	struct mcb_label *begin, *cont, *end;

	begin = mcb_declare_label(NULL, ctx->fn);
	cont = mcb_declare_label(NULL, ctx->fn);
	end = mcb_declare_label(NULL, ctx->fn);

	if (mcb_inst_define_label(begin, ctx->fn))
		panic("mcb_inst_define_label()");
	cmp_result = compile_cmp_expr(stmt->cond, ctx);
	if (mcb_inst_branch(cmp_result, cont, end, ctx->fn))
		panic("mcb_inst_branch()");
	if (mcb_inst_define_label(cont, ctx->fn))
		panic("mcb_inst_define_label()");

	for (size_t i = 0; i < stmt->stmts_count; i++) {
		if (compile_stmt(stmt->stmts[i], ctx))
			return 1;
	}

	if (mcb_inst_jmp(begin, ctx->fn))
		panic("mcb_inst_jmp()");

	if (mcb_inst_define_label(end, ctx->fn))
		panic("mcb_inst_define_label()");
	return 0;
}
