/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_COMPILER_LET_H
#define CZAKOC_COMPILER_LET_H
#include "compiler.h"
#include "../let.h"

int compile_let_stmt(
		struct zako_let_stmt *stmt,
		struct compiler_context *ctx);

#endif
