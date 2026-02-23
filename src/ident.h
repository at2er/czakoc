/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_IDENT_H
#define CZAKOC_IDENT_H
#include "type.h"

enum ZAKO_IDENT_KIND {
	FN_IDENT
};

struct zako_ident {
	char *name;
	enum ZAKO_IDENT_KIND kind;
	struct zako_type *type;
};

void free_zako_ident(struct zako_ident *self);

#endif
