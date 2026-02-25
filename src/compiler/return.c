/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include <mcb/value.h>
#include <mcb/inst/ret.h>
#include "compiler.h"
#include "expr.h"
#include "return.h"
#include "../panic.h"
#include "../return.h"

int
compile_return_stmt(
		struct zako_return_stmt *stmt,
		struct compiler_context *ctx)
{
	struct mcb_value *result;
	assert(stmt && ctx);
	result = compile_expr(stmt->expr, ctx);
	if (mcb_inst_ret(result, ctx->fn))
		panic("mcb_inst_ret()");
	return 0;
}
