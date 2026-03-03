/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_COMPILER_WHILE_H
#define CZAKOC_COMPILER_WHILE_H
#include "compiler.h"
#include "../while.h"

int compile_while_stmt(
		struct zako_while_stmt *stmt,
		struct compiler_context *ctx);

#endif
