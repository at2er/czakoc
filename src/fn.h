/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_FN_H
#define CZAKOC_FN_H

struct zako_ident;
struct zako_type;
struct zako_fn_type {
	int argc;
	struct zako_ident **args;
	struct zako_type *type;
};

void destory_zako_fn_type(struct zako_fn_type *self);

#endif
