/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_H
#define CZAKOC_H
#include <stdint.h>
#include <sys/stat.h>

enum CZAKOC_FLAG {
	CZAKOC_OUTPUT_AST          = 1,
	CZAKOC_OUTPUT_IR           = 1 << 2,
	CZAKOC_OUTPUT_LEXER_TOKENS = 1 << 3,
};

/* defined in main.c and don't change from other place */
extern mode_t czakoc_file_mode;
extern uint64_t czakoc_flags;
extern const char *czakoc_cache_dir;
extern const char *czakoc_cwd;
extern const char *czakoc_root_mod;

#endif
