/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdlib.h>
#include "fn.h"
#include "type.h"

void
free_zako_type(struct zako_type *self)
{
	if (!self)
		return;
	destory_zako_fn_type(&self->inner.fn);
	free(self);
}
