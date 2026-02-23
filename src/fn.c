/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdlib.h>
#include "fn.h"
#include "ident.h"
#include "type.h"

void
destory_zako_fn_type(struct zako_fn_type *self)
{
	for (int i = 0; i < self->argc; i++)
		free_zako_ident(self->args[i]);
	free(self->args);
}
