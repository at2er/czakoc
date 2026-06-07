/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_ZAKO_H
#define CZAKOC_ZAKO_H

enum ZK_TYPE_BASE {
	ZK_8, ZK_16, ZK_32, ZK_64,

	ZK_FN
};

struct zk_type {
	enum ZK_TYPE_BASE base;
	unsigned int
			_signed:1;
};

struct zk_ident {
	char *name;
	struct zk_type typ;
};

#endif
