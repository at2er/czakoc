/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_COMPILER_FN_H
#define CZAKOC_COMPILER_FN_H
#include <mcb/func.h>
#include <mcb/value.h>
#include "compiler.h"
#include "../fn.h"

struct mcb_value *compile_fn_call(
		struct zako_fn_call *call,
		struct compiler_context *ctx);
struct mcb_func *compile_fn_declaration(
		struct zako_fn_declaration *declaration,
		struct compiler_context *ctx);
int compile_fn_definition(
		struct zako_fn_definition *definition,
		struct compiler_context *ctx);

#endif
