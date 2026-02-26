/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_SEMANTICS_IF_H
#define CZAKOC_SEMANTICS_IF_H
#include "semantics.h"
#include "../if.h"
#include "../type.h"

enum ANALYSIS_RESULT analyse_if_stmt(struct zako_if_stmt *stmt);

#endif
