/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_COMPILER_VALUE_H
#define CZAKOC_COMPILER_VALUE_H
#include <mcb/value.h>
#include "compiler.h"
#include "../value.h"

struct mcb_value *compile_value(
		struct zako_value *value,
		struct compiler_context *ctx);

#endif
