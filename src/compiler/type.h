/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_COMPILER_TYPE_H
#define CZAKOC_COMPILER_TYPE_H
#include <mcb/type.h>
#include "../type.h"

const struct mcb_type *mcb_type_from_zako(struct zako_type *type);

#endif
