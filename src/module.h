/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_MODULE_H
#define CZAKOC_MODULE_H
#include "path_max.h"

struct zako_module {
	/* Prefix of identifiers in this module. */
	char *prefix;

	/* Every file is a module, and you can't
	 * define many modules in a single file. */
	char file_path[PATH_MAX];
};

#endif
