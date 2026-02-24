/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include <mcb/context.h>
#include <mcb/func.h>
#include <mcb/inst/add.h>
#include <mcb/inst/call.h>
#include <mcb/inst/div.h>
#include <mcb/inst/mul.h>
#include <mcb/inst/ret.h>
#include <mcb/inst/store.h>
#include <mcb/inst/sub.h>
#include <mcb/target/gnu_asm.h>
#include <mcb/value.h>
#include <stddef.h>
#include <stdio.h>
#include "compiler.h"
#define UTILSH_CONTAINER_OF_STRIP
#include "container_of.h"
#include "darr.h"
#include "ealloc.h"
#include "ident.h"
#include "panic.h"
#include "parser.h"
#include "type.h"

struct compiler_context {
	struct mcb_func *fn;
	struct mcb_context mcb;
	struct zako_module *mod;
};

static struct mcb_value *compile_binary_expr(
		struct zako_binary_expr *binary,
		struct compiler_context *ctx);
static struct mcb_value *compile_expr(
		struct zako_expr *expr,
		struct compiler_context *ctx);
static struct mcb_value *compile_fn_call(
		struct zako_fn_call *call,
		struct compiler_context *ctx);
static int compile_fn_definition(
		struct zako_fn_definition *definition,
		struct compiler_context *ctx);
static struct mcb_value *compile_primary_expr(
		struct zako_value *primary,
		struct compiler_context *ctx);
static int compile_return_stmt(
		struct zako_return_stmt *stmt,
		struct compiler_context *ctx);
static int compile_stmt(
		struct zako_stmt *stmt,
		struct compiler_context *ctx);
static int compile_toplevel_stmt(
		struct zako_toplevel_stmt *stmt,
		struct compiler_context *ctx);
static struct mcb_value *compile_value(
		struct zako_value *value,
		struct compiler_context *ctx);
static enum MCB_TYPE mcb_type_from_zako(struct zako_type *type);

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

struct mcb_value *
compile_primary_expr(
		struct zako_value *primary,
		struct compiler_context *ctx)
{
	assert(primary && ctx);
	return compile_value(primary, ctx);
}

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

int
compile_stmt(struct zako_stmt *stmt, struct compiler_context *ctx)
{
	assert(stmt && ctx);
	switch (stmt->kind) {
	case RETURN_STMT:
		return compile_return_stmt(stmt->inner.return_stmt, ctx);
	}
	return 0;
}

int
compile_toplevel_stmt(
		struct zako_toplevel_stmt *stmt,
		struct compiler_context *ctx)
{
	assert(stmt && ctx);
	switch (stmt->kind) {
	case FN_DECLARATION:
		panic("toplevel declaration statement");
		break;
	case FN_DEFINITION:
		return compile_fn_definition(
				stmt->inner.fn_definition,
				ctx);
	}
	return 0;
}

struct mcb_value *
compile_value(struct zako_value *value, struct compiler_context *ctx)
{
	struct mcb_value *mcb_val;
	switch (value->kind) {
	case EXPR_VALUE:
		return compile_expr(value->data.expr, ctx);
	case FN_CALL_VALUE:
		return compile_fn_call(value->data.fn_call, ctx);
	case IDENT_VALUE:
		return value->data.ident->value;
	case INT_LITERAL:
		mcb_val = mcb_define_value(
				"_",
				mcb_type_from_zako(value->type),
				ctx->fn);
		if (!mcb_val)
			panic("mcb_define_value()");
		if (mcb_inst_store_int(mcb_val, value->data.i, ctx->fn))
			panic("mcb_inst_store_int()");
		break;
	}
	return mcb_val;
}

enum MCB_TYPE
mcb_type_from_zako(struct zako_type *type)
{
	assert(type);

	switch (type->builtin) {
	case I8_TYPE:  return MCB_I8;  case I16_TYPE: return MCB_I16;
	case I32_TYPE: return MCB_I32; case I64_TYPE: return MCB_I64;
	case U8_TYPE:  return MCB_U8;  case U16_TYPE: return MCB_U16;
	case U32_TYPE: return MCB_U32; case U64_TYPE: return MCB_U64;
	default: break;
	}

	abort();
	return -1;
}

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
