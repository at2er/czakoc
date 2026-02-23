/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdlib.h>
#include "ident.h"

void
free_zako_ident(struct zako_ident *self)
{
	free(self->name);
	free_zako_type(self->type);
	free(self);
}
