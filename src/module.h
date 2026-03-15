/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_MODULE_H
#define CZAKOC_MODULE_H
#include "jim2.h"
#include "path_max.h"

struct zako_module {
	/* Prefix of identifiers in this module. */
	char *prefix;

	/* Every file is a module, and you can't
	 * define many modules in a single file. */
	char file_path[PATH_MAX];

	struct zako_scope *scope;
};

struct zako_module_import {
	struct zako_module *mod;
	/* e.g. std.write
	 *      ^^^       */
	char *name;
};

struct zako_module *create_module(const char *path);
void free_module(struct zako_module *self);
void free_module_import(struct zako_module_import *self);
char *gen_mod_prefix(const char *path);
void print_module_import(struct zako_module_import *self, Jim *jim);

#endif
