/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_SEMANTICS_FN_H
#define CZAKOC_SEMANTICS_FN_H
#include "semantics.h"
#include "../fn.h"
#include "../type.h"

enum ANALYSIS_RESULT analyse_fn_call(
		struct zako_fn_call *call,
		struct zako_type *expect_type);

#endif
