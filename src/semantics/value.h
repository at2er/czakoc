/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_SEMANTICS_VALUE_H
#define CZAKOC_SEMANTICS_VALUE_H
#include "semantics.h"
#include "../type.h"
#include "../value.h"

enum ANALYSIS_RESULT analyse_value(
		struct zako_value *value,
		struct zako_type *expect_type);

#endif
