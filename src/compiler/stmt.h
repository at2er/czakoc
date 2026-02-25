/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_COMPILER_STMT_H
#define CZAKOC_COMPILER_STMT_H
#include "compiler.h"
#include "../stmt.h"

int compile_stmt(
		struct zako_stmt *stmt,
		struct compiler_context *ctx);
int compile_toplevel_stmt(
		struct zako_toplevel_stmt *stmt,
		struct compiler_context *ctx);

#endif
