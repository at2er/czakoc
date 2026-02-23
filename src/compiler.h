/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_COMPILER_H
#define CZAKOC_COMPILER_H
#include <stddef.h>
#include "module.h"
#include "parser.h"

int compile_file(
		struct zako_toplevel_stmt **stmts,
		size_t stmts_count,
		struct zako_module *mod);

#endif
