/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "if.h"
#include "../ealloc.h"
#include "../if.h"

struct zako_stmt *
parse_if_stmt(struct parser *parser)
{
	struct zako_if_stmt *self;
	struct zako_stmt *stmt;
	assert(parser);
	stmt = ecalloc(1, sizeof(*stmt));
	stmt->kind = IF_STMT;
	stmt->inner.if_stmt = self = ecalloc(1, sizeof(*self));
	return NULL;
}
