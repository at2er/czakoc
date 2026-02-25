/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_SEMANTICS_H
#define CZAKOC_SEMANTICS_H
#include <stdbool.h>
#include "../expr.h"
#include "../type.h"

enum ANALYSIS_RESULT {
	SUCCESS,
	UNKOWN_ANALYSIS_ERR,
	TYPE_COMPARE_EXPECT_SIGNED,
	TYPE_COMPARE_EXPECT_UNSIGNED,
	TYPE_COMPARE_IMPLICIT_CAST
};

enum ANALYSIS_RESULT analyse_expr(
		struct zako_expr *expr,
		struct zako_type *expect_type);

const char *cstr_analysis_result(enum ANALYSIS_RESULT result);

#endif
