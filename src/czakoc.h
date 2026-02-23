/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_H
#define CZAKOC_H
#include <stdint.h>

enum CZAKOC_FLAG {
	CZAKOC_OUTPUT_AST = 1,
	CZAKOC_OUTPUT_LEXER_TOKENS = 1 << 1
};

/* defined in main.c and don't change from other place */
extern uint64_t czakoc_flags;

#endif
