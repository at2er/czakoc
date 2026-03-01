/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include <mcb/type.h>
#include <stdlib.h>
#include "utils.h"
#include "../ealloc.h"
#include "../panic.h"
#include "../type.h"

static enum MCB_BUILTIN_TYPE mcb_builtin_type_from_zako(
		enum ZAKO_BUILTIN_TYPE builtin);

enum MCB_BUILTIN_TYPE
mcb_builtin_type_from_zako(enum ZAKO_BUILTIN_TYPE builtin)
{
	switch (builtin) {
	case I8_TYPE:  return MCB_I8;  case I16_TYPE: return MCB_I16;
	case I32_TYPE: return MCB_I32; case I64_TYPE: return MCB_I64;
	case U8_TYPE:  return MCB_U8;  case U16_TYPE: return MCB_U16;
	case U32_TYPE: return MCB_U32; case U64_TYPE: return MCB_U64;
	case ARR_TYPE: return MCB_ARRAY;
	default:
		break;
	}
	panic("mcb_builtin_type_from_zako()");
	return -1;
}

const struct mcb_type *
mcb_type_from_zako(struct zako_type *type)
{
	enum MCB_BUILTIN_TYPE mcb_builtin;
	struct mcb_type *mcb_type;
	const struct mcb_type *mcb_type_builtin;
	assert(type);

	mcb_builtin = mcb_builtin_type_from_zako(type->builtin);
	mcb_type_builtin = mcb_get_type_from_builtin(mcb_builtin);
	if (mcb_type_builtin)
		return mcb_type_builtin;

	type->mcb_type = mcb_type = ecalloc(1, sizeof(*mcb_type));
	mcb_type->builtin = mcb_builtin;
	switch (type->builtin) {
	case ARR_TYPE:
		mcb_type->inner = (struct mcb_type*)mcb_type_from_zako(
				type->inner.arr.elem_type);
		break;
	default:
		panic("type->builtin");
		break;
	}

	return mcb_type;
}
