/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_COMPILER_EXPR_H
#define CZAKOC_COMPILER_EXPR_H
#include <mcb/value.h>
#include "compiler.h"
#include "../expr.h"

struct mcb_value *compile_expr(
		struct zako_expr *expr,
		struct compiler_context *ctx);

#endif
