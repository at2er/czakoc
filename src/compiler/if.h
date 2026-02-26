/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_COMPILER_IF_H
#define CZAKOC_COMPILER_IF_H
#include "compiler.h"
#include "../if.h"

int compile_if_stmt(
		struct zako_if_stmt *stmt,
		struct compiler_context *ctx);

#endif
