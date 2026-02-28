/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_IDENT_H
#define CZAKOC_IDENT_H
#include <mcb/value.h>
#include <stdbool.h>
#include "type.h"

enum ZAKO_IDENT_KIND {
	FN_IDENT,
	FN_ARG_IDENT
};

struct zako_ident {
	char *name;
	enum ZAKO_IDENT_KIND kind;
	struct zako_type *type;

	bool mutable;

	struct mcb_value *value;
};

void free_ident(struct zako_ident *self);
void print_ident(struct zako_ident *self, Jim *jim);

#endif
