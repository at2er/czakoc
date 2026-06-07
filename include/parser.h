/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_PARSER_H
#define CZAKOC_PARSER_H

struct zako_type;
struct zako_ident_sign {
	char *ident;
	struct zako_type *type;
};

struct zako_type {
	enum ZAKO_TYPE base;
	struct zako_type *ext;
}

#endif
