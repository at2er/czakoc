/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_FN_H
#define CZAKOC_FN_H
#include <mcb/func.h>

struct zako_ident;
struct zako_type;
struct zako_fn_type {
	int argc;
	struct zako_ident **args;
	struct zako_type *type;

	struct mcb_func *mcb_fn;
};

void destory_zako_fn_type(struct zako_fn_type *self);

#endif
