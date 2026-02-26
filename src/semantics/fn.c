/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include "fn.h"
#include "semantics.h"
#include "value.h"
#include "../fn.h"
#include "../ident.h"
#include "../type.h"

enum ANALYSIS_RESULT
analyse_fn_call(struct zako_fn_call *call, struct zako_type *expect_type)
{
	struct zako_fn_type *fn_type;
	int ret;
	assert(call && expect_type);
	fn_type = &call->fn->type->inner.fn;
	for (int i = 0; i < call->argc; i++) {
		ret = analyse_value(call->args[i], fn_type->args[i]->type);
		if (ret)
			return ret;
	}
	return compare_builtin_type(call->fn->type->inner
			.fn.type->builtin,
			expect_type->builtin);
}
