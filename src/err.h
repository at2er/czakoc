/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_ERR_H
#define CZAKOC_ERR_H
#include <stdio.h>

#define ERR_LOC "\x1b[1m%s:%d:%d:\x1b[0m "
#define ERR_LOC_ARGS(TOK_REF) \
	(TOK_REF)->loc.fpath, \
	(int)(TOK_REF)->loc.line, \
	(int)(TOK_REF)->loc.column
#define ERR_MSG(...)      "czakoc: \x1b[1m\x1b[31merror: \x1b[0m" __VA_ARGS__ "\n"
#define ERR_MSG_CONT(...) "      | " __VA_ARGS__ "\n"

#define print_err_msg(MSG) printf(ERR_MSG(MSG))
#define printf_err_msg(FMT, ...) printf(ERR_MSG(FMT), __VA_ARGS__)

#define print_err(MSG, TOK_REF) \
	printf(ERR_MSG(ERR_LOC MSG), ERR_LOC_ARGS(TOK_REF))
#define printf_err(FMT, TOK_REF, ...) \
	printf(ERR_MSG(ERR_LOC FMT), ERR_LOC_ARGS(TOK_REF), __VA_ARGS__)

#define print_err_cont(MSG) \
	printf(ERR_MSG_CONT(MSG))
#define printf_err_cont(FMT, ...) \
	printf(ERR_MSG_CONT(FMT), __VA_ARGS__)

#endif
