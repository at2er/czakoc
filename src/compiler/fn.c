/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <mcb/value.h>
#include <mcb/func.h>
#include <mcb/inst/call.h>
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

int
compile_fn_definition(
		struct zako_fn_definition *definition,
		struct compiler_context *ctx)
{
	struct zako_fn_declaration *declaration;
	struct zako_fn_type *fn_type;
	struct mcb_func *mcb_fn;
	struct mcb_func_arg *mcb_fn_arg;
	assert(definition && ctx);

	declaration = definition->declaration;
	fn_type = &declaration->ident->type->inner.fn;

	mcb_fn = mcb_define_func(
			declaration->ident->name,
			mcb_type_from_zako(fn_type->type),
			declaration->public ? MCB_EXPORT_FUNC : MCB_LOCAL_FUNC,
			&ctx->mcb);
	if (!mcb_fn)
		goto err_define_func;
	fn_type->mcb_fn = mcb_fn;
	for (int i = 0; i < fn_type->argc; i++) {
		mcb_fn_arg = mcb_define_func_arg(
				fn_type->args[i]->name,
				mcb_type_from_zako(fn_type->args[i]->type),
				mcb_fn);
		if (!mcb_fn_arg)
			goto err_define_arg;
		fn_type->args[i]->value = mcb_define_value_from_func_arg(
				fn_type->args[i]->name,
				mcb_fn_arg,
				mcb_fn);
	}
	ctx->fn = mcb_fn;
	for (size_t i = 0; i < definition->stmts_count; i++) {
		if (compile_stmt(definition->stmts[i], ctx))
			return 1;
	}
	return 0;
err_define_func:
	panic("mcb_define_func()");
	return 1;
err_define_arg:
	panic("mcb_define_func_arg()");
	return 1;
}
