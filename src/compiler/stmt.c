/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include "compiler.h"
#include "fn.h"
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
		break; // TODO
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
