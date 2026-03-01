/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <mcb/value.h>
#include <mcb/func.h>
#include <mcb/inst/call.h>
#include <mcb/syscall.h>
#include "compiler.h"
#include "stmt.h"
#include "utils.h"
#include "value.h"
#include "../ealloc.h"
#include "../fn.h"
#include "../ident.h"
#include "../panic.h"
#include "../type.h"

struct mcb_value *
compile_fn_call(struct zako_fn_call *call, struct compiler_context *ctx)
{
	struct mcb_value *value;
	struct mcb_value **args;
	assert(call && ctx);
	value = mcb_define_value(
			"fn_call_result",
			mcb_type_from_zako(call->fn->type->inner.fn.type),
			ctx->fn);
	args = ecalloc(call->argc, sizeof(*args));
	for (int i = 0; i < call->argc; i++) {
		args[i] = compile_value(call->args[i], ctx);
		if (!args[i])
			panic("compile_value()");
	}
	if (mcb_inst_call(value,
				call->fn->type->inner.fn.mcb_fn,
				call->argc,
				args,
				ctx->fn))
		panic("mcb_inst_call()");
	free(args);
	return value;
}

struct mcb_func *
compile_fn_declaration(
		struct zako_fn_declaration *declaration,
		struct compiler_context *ctx)
{
	struct zako_fn_type *fn_type;
	struct mcb_func *mcb_fn;
	assert(declaration && ctx);

	fn_type = &declaration->ident->type->inner.fn;

	mcb_fn = mcb_define_func(
			declaration->ident->name,
			mcb_type_from_zako(fn_type->type),
			declaration->public ? MCB_EXPORT_FUNC : MCB_LOCAL_FUNC,
			&ctx->mcb);
	if (!mcb_fn)
		panic("mcb_define_func()");
	if (fn_type->syscall >= 0)
		if (mcb_func_to_syscall(fn_type->syscall, mcb_fn))
			panic("mcb_func_to_syscall()");

	fn_type->mcb_fn = mcb_fn;

	for (int i = 0; i < fn_type->argc; i++) {
		fn_type->args[i]->value = mcb_define_value(
				fn_type->args[i]->name,
				mcb_type_from_zako(fn_type->args[i]->type),
				mcb_fn);
		if (!fn_type->args[i]->value)
			panic("mcb_define_func_arg()");
		if (mcb_append_func_arg(fn_type->args[i]->value, mcb_fn))
			panic("mcb_append_func_arg()");
	}

	return mcb_fn;
}

int
compile_fn_definition(
		struct zako_fn_definition *definition,
		struct compiler_context *ctx)
{
	assert(definition && ctx);

	ctx->fn = compile_fn_declaration(definition->declaration, ctx);
	if (!ctx->fn)
		return 1;

	for (size_t i = 0; i < definition->stmts_count; i++) {
		if (compile_stmt(definition->stmts[i], ctx))
			return 1;
	}
	return 0;
}
