/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_COMPILER_H
#define CZAKOC_COMPILER_H
#include <mcb/context.h>
#include <mcb/func.h>
#include <stddef.h>
#include "../module.h"
#include "../stmt.h"

struct compiler_context {
	struct mcb_func *fn;
	struct mcb_context mcb;
	struct zako_module *mod;
};

int compile_file(
		struct zako_toplevel_stmt **stmts,
		size_t stmts_count,
		struct zako_module *mod);

#endif
