/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_SEMANTICS_H
#define CZAKOC_SEMANTICS_H
#include <stdbool.h>
#include "parser.h"
#include "type.h"

int analyse_expr(struct zako_expr *expr, struct zako_type *expect_type);

#endif
