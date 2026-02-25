/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_PANIC_H
#define CZAKOC_PANIC_H
#include <stdio.h>
#include <stdlib.h>

#define PANIC_FMT "czakoc: \x1b[41mpanic\x1b[0m: \x1b[1m%s:%s:%d\x1b[0m: "
#define PANIC_FMT_ARG __FILE__, __func__, __LINE__
#define panic(MSG) do { \
	printf(PANIC_FMT MSG"\n", PANIC_FMT_ARG); \
	abort(); \
} while (0);
#define panicf(FMT, ...) do { \
	printf(PANIC_FMT FMT"\n", PANIC_FMT_ARG, __VA_ARGS__); \
	abort(); \
} whlie (0);


#endif
