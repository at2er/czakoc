/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_TYPE_H
#define CZAKOC_TYPE_H
#include <stdint.h>
#include "fn.h"

#define IS_NUMBER(BUILTIN_TYPE) \
	(IS_SIGNED_NUMBER(BUILTIN_TYPE) || IS_UNSIGNED_NUMBER(BUILTIN_TYPE))
#define IS_SIGNED_NUMBER(BUILTIN_TYPE) \
	((BUILTIN_TYPE) >= I8_TYPE && (BUILTIN_TYPE) <= I64_TYPE)
#define IS_UNSIGNED_NUMBER(BUILTIN_TYPE) \
	((BUILTIN_TYPE) >= U8_TYPE && (BUILTIN_TYPE) <= U64_TYPE)

enum ZAKO_BUILTIN_TYPE {
	I8_TYPE, I16_TYPE, I32_TYPE, I64_TYPE,
	U8_TYPE, U16_TYPE, U32_TYPE, U64_TYPE,

	/* virtual types, can't be compiled to machine code */
	CMP_EXPR_TYPE, FN_TYPE
};

struct zako_type {
	enum ZAKO_BUILTIN_TYPE builtin;
	union {
		struct zako_fn_type fn;
	} inner;
};

void free_zako_type(struct zako_type *self);
enum ZAKO_BUILTIN_TYPE get_builtin_type_from_int_literal(int64_t i);
void print_type(struct zako_type *self, Jim *jim);

#endif
