/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include "expr.h"
#include "if.h"
#include "semantics.h"

static int analyse_if_cond(struct zako_expr *cond);

int
analyse_if_cond(struct zako_expr *cond)
{
	assert(cond);
	return analyse_cmp_expr(cond);
}

enum ANALYSIS_RESULT
analyse_if_stmt(struct zako_if_stmt *stmt)
{
	int ret;
	assert(stmt);
	ret = analyse_if_cond(stmt->cond);
	if (ret)
		return ret;
	return 0;
}
