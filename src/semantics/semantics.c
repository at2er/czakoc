/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include "semantics.h"
#include "../type.h"

static const char *analysis_result_str[] = {
	[SUCCESS] = "analysis succeed",
	[UNKOWN_ANALYSIS_ERR] = "unkown analysis error",
	[TYPE_COMPARE_EXPECT_SIGNED] =
		"type compare failed: expect signed integer",
	[TYPE_COMPARE_EXPECT_UNSIGNED] =
		"type compare failed: expect unsigned integer",
	[TYPE_COMPARE_IMPLICIT_CAST] =
		"type compare failed: implicit cast"
};

enum ANALYSIS_RESULT
compare_builtin_type(enum ZAKO_BUILTIN_TYPE src, enum ZAKO_BUILTIN_TYPE expect)
{
	if (expect == src)
		return SUCCESS;
	if (IS_SIGNED_NUMBER(expect)) {
		if (!IS_SIGNED_NUMBER(src))
			return TYPE_COMPARE_EXPECT_SIGNED;
		if (src <= expect)
			return SUCCESS;
		return TYPE_COMPARE_IMPLICIT_CAST;
	}
	if (IS_UNSIGNED_NUMBER(expect)) {
		if (!IS_UNSIGNED_NUMBER(src))
			return TYPE_COMPARE_EXPECT_UNSIGNED;
		if (src <= expect)
			return SUCCESS;
		return TYPE_COMPARE_IMPLICIT_CAST;
	}
	return UNKOWN_ANALYSIS_ERR;
}

const char *
cstr_analysis_result(enum ANALYSIS_RESULT result)
{
	return analysis_result_str[result];
}
