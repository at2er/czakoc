/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_SEMANTICS_IDENT_H
#define CZAKOC_SEMANTICS_IDENT_H
#include "semantics.h"
#include "../ident.h"
#include "../type.h"

enum ANALYSIS_RESULT analyse_ident(
		struct zako_ident *ident,
		struct zako_type *expect_type);

#endif
