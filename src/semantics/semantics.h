/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_SEMANTICS_H
#define CZAKOC_SEMANTICS_H
#include "../type.h"

enum ANALYSIS_RESULT {
	SUCCESS,
	UNKOWN_ANALYSIS_ERR,
	TYPE_COMPARE_EXPECT_SIGNED,
	TYPE_COMPARE_EXPECT_UNSIGNED,
	TYPE_COMPARE_IMPLICIT_CAST
};

enum ANALYSIS_RESULT compare_builtin_type(
		enum ZAKO_BUILTIN_TYPE src,
		enum ZAKO_BUILTIN_TYPE expect);

const char *cstr_analysis_result(enum ANALYSIS_RESULT result);

#endif
