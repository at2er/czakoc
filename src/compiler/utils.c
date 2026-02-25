/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include <mcb/type.h>
#include <stdlib.h>
#include "utils.h"
#include "../type.h"

enum MCB_TYPE
mcb_type_from_zako(struct zako_type *type)
{
	assert(type);

	switch (type->builtin) {
	case I8_TYPE:  return MCB_I8;  case I16_TYPE: return MCB_I16;
	case I32_TYPE: return MCB_I32; case I64_TYPE: return MCB_I64;
	case U8_TYPE:  return MCB_U8;  case U16_TYPE: return MCB_U16;
	case U32_TYPE: return MCB_U32; case U64_TYPE: return MCB_U64;
	default: break;
	}

	abort();
	return -1;
}
