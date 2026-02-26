/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_SEMANTICS_EXPR_H
#define CZAKOC_SEMANTICS_EXPR_H
#include "semantics.h"
#include "../expr.h"
#include "../type.h"

enum ANALYSIS_RESULT analyse_cmp_expr(struct zako_expr *expr);
enum ANALYSIS_RESULT analyse_expr(
		struct zako_expr *expr,
		struct zako_type *expect_type);

#endif
