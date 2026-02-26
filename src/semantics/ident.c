/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include "ident.h"
#include "semantics.h"
#include "../ident.h"
#include "../type.h"

enum ANALYSIS_RESULT
analyse_ident(struct zako_ident *ident, struct zako_type *expect_type)
{
	assert(ident && expect_type);
	return compare_builtin_type(ident->type->builtin, expect_type->builtin);
}
