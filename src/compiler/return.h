/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_COMPILER_RETURN_H
#define CZAKOC_COMPILER_RETURN_H
#include "compiler.h"
#include "../return.h"

int compile_return_stmt(
		struct zako_return_stmt *stmt,
		struct compiler_context *ctx);

#endif
