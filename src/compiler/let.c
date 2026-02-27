/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include <mcb/inst/alloc_var.h>
#include <mcb/inst/store.h>
#include <mcb/value.h>
#include "compiler.h"
#include "expr.h"
#include "let.h"
#include "utils.h"
#include "../panic.h"
#include "../let.h"

int
compile_let_stmt(
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
	stmt->ident->value = container;
	return 0;
}
