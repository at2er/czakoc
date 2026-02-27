/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include "compiler.h"
#include "fn.h"
#include "if.h"
#include "let.h"
#include "return.h"
#include "stmt.h"
#include "../panic.h"
#include "../stmt.h"

int
compile_stmt(struct zako_stmt *stmt, struct compiler_context *ctx)
{
	assert(stmt && ctx);
	switch (stmt->kind) {
	case IF_STMT:
		return compile_if_stmt(stmt->inner.if_stmt, ctx);
	case LET_STMT:
		return compile_let_stmt(stmt->inner.let_stmt, ctx);
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
