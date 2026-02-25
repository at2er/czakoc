/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include <mcb/context.h>
#include <mcb/target/gnu_asm.h>
#include <mcb/value.h>
#include <stddef.h>
#include <stdio.h>
#include "compiler.h"
#include "stmt.h"


int
compile_file(
		struct zako_toplevel_stmt **stmts,
		size_t stmts_count,
		struct zako_module *mod)
{
	struct compiler_context ctx = {0};
	assert(stmts && mod);

	mcb_define_context(&ctx.mcb);
	ctx.mod = mod;

	for (size_t i = 0; i < stmts_count; i++) {
		if (compile_toplevel_stmt(stmts[i], &ctx))
			goto err_destory_ctx;
	}

	if (mcb_target_gnu_asm(stdout, &ctx.mcb))
		goto err_destory_ctx;

	mcb_destory_context(&ctx.mcb);

	return 0;
err_destory_ctx:
	mcb_destory_context(&ctx.mcb);
	return 1;
}
