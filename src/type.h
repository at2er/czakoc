/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_TYPE_H
#define CZAKOC_TYPE_H
#include "fn.h"

enum ZAKO_BUILTIN_TYPE {
	I8_TYPE, I16_TYPE, I32_TYPE, I64_TYPE,
	U8_TYPE, U16_TYPE, U32_TYPE, U64_TYPE,

	/* virtual types, can't be compiled to machine code */
	FN_TYPE
};

struct zako_type {
	enum ZAKO_BUILTIN_TYPE builtin;
	union {
		struct zako_fn_type fn;
	} inner;
};

const char *cstr_from_builtin_type(enum ZAKO_BUILTIN_TYPE type);

void free_zako_type(struct zako_type *self);

#endif
