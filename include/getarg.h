/** Simple arguments parser for C
 *
 * Put 'SCTUI_IMPL' to one source file to compile it and use it.
 *
 * Usage:
 * - define options:
 *     ```
 *     static struct option opts[] = {
 *       // OPT_FLAG
 *       OPT_FLAG("flag-a-long-name", 'a', &flags, 1),
 *       OPT_FLAG("flag-b-long-name", 'b', &flags, 1 << 1),
 *
 *       // OPT_HELP
 *       // @param usages: const char * [], see `Help (Usages)` for more
 *       OPT_HELP("help", 'h', usages),
 *       ```
 *           static const char *usages[] = {
 *           "usage: getarg: [OPTIONS]...",
 *           "",
 *           "options:",
 *           "...OPTIONS...",
 *           NULL
 *           };
 *       ```
 *
 *       // OPT_MANUAL
 *       // @param handler: getarg_opt_func
 *       OPT_MANUAL("define", 'd' define_handler),
 *       ```
 *           enum GETARG_RESULT
 *           define_handler(int *argc, char **argv[], struct option *opt)
 *           {
 *               const char *name, *content;
 *
 *               name = **argv;
 *               GETARG_SHIFT(*argc, *argv);
 *
 *               content = **argv;
 *               GETARG_SHIFT(*argc, *argv);
 *
 *               return GETARG_RESULT_SUCCESSFUL;
 *           }
 *       ```
 *
 *       // OPT_STRING
 *       // @param str: char ** (point to 'char *')
 *       OPT_STRING("string", 's', &str),
 *
 *       // OPT_UINT
 *       // @param uint: uint64_t * (point to 'uint64_t')
 *       OPT_UINT("uint", 'u', &uint),
 *
 *       OPT_END
 *     }
 *     ```
 *
 * - parsing:
 *     ```in 'main()'
 *     enum GETARG_RESULT ret;
 *
 *     GETARG_BEGIN(ret, argc, argv, opts) {
 *     case GETARG_RESULT_SUCCESSFUL: break;
 *     case GETARG_RESULT_UNKNOWN:    GETARG_SHIFT(argc, argv); break;
 *     // ... handle more result (enum GETARG_RESULT)
 *     default: return 1;
 *     } GETARG_END;
 *     ```
 *
 * Version:
 *     1.1.0: OPT_MANUAL for manual parsing by user function.
 *     1.0.0: change to be a single-header only library.
 *
 * MIT License
 *
 * Copyright (c) 2025 at2er <xb0515@outlook.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef LIBGETARG_H
#define LIBGETARG_H
#include <stddef.h>
#include <stdint.h>

#define NO_LONG_NAME NULL
#define NO_SHORT_NAME '\0'

enum GETARG_OPT_TYPE {
	GETARG_OPT_END,

	GETARG_OPT_FLAG, /* --enable-xxx without any argument */
	GETARG_OPT_HELP,
	GETARG_OPT_MANUAL, /* manually parse arguments by function */
	GETARG_OPT_STRING,
	GETARG_OPT_UINT /* uint64_t */
};

enum GETARG_RESULT {
	GETARG_RESULT_END,

	GETARG_RESULT_APPLIED_HELP_OPT,
	GETARG_RESULT_DASH,
	GETARG_RESULT_DOUBLE_DASH,
	GETARG_RESULT_OPT_NOT_FOUND,
	GETARG_RESULT_PARSE_ARG_FAILED,
	GETARG_RESULT_SUCCESSFUL,
	GETARG_RESULT_UNKNOWN
};

struct option;
typedef enum GETARG_RESULT (*getarg_opt_func)(
		int *argc,
		char **argv[],
		struct option *opt);

struct option {
	enum GETARG_OPT_TYPE type;
	const char *long_name;
	const char short_name;
	union {
		void *v;
		getarg_opt_func f;
	} value;
	uintptr_t data;
};

#define OPT_FLAG(LN, SN, FLAGS, FLAG_BIT) \
	{GETARG_OPT_FLAG, LN, SN, {.v = FLAGS}, FLAG_BIT}
#define OPT_HELP(LN, SN, USAGES)    {GETARG_OPT_HELP,   LN, SN, {.v = USAGES},  0}
#define OPT_MANUAL(LN, SN, HANDLER) {GETARG_OPT_MANUAL, LN, SN, {.f = HANDLER}, 0}
#define OPT_STRING(LN, SN, STR)     {GETARG_OPT_STRING, LN, SN, {.v = STR},     0}
#define OPT_UINT(LN, SN, UINT)      {GETARG_OPT_UINT,   LN, SN, {.v = UINT},    0}
#define OPT_END {GETARG_OPT_END, NO_LONG_NAME, NO_SHORT_NAME, {.v = NULL},  0}

#define GETARG_BEGIN(RESULT, ARGC, ARGV, OPTS) do { \
	GETARG_SHIFT(ARGC, ARGV); \
	while (((RESULT) = getarg(&(ARGC), &(ARGV), (OPTS))) != GETARG_RESULT_END) { \
		switch ((RESULT))
#define GETARG_END \
	}} while (0)
#define GETARG_SHIFT(ARGC, ARGV) do { (ARGC)--; (ARGV)++; } while (0)

enum GETARG_RESULT getarg(int *argc, char **argv[], struct option *opts);

#endif

#ifdef GETARG_IMPL
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _GETARG_SHIFT(ARGC_PTR, ARGV_PTR) GETARG_SHIFT(*(ARGC_PTR), *(ARGV_PTR))

#define _GETARG_FIND_OPT_TEMPLATE(OPTS, COND) \
	{ \
		for (int i = 0; (OPTS)[i].type != GETARG_OPT_END; i++) { \
			if (COND) \
				return &opts[i]; \
		} \
		return NULL; \
	}

static struct option *
_getarg_find_long_opt(char *opt, struct option *opts)
	_GETARG_FIND_OPT_TEMPLATE(opts, (opts[i].long_name
				&& strcmp(&opt[2], opts[i].long_name)
				== 0))

static struct option *
_getarg_find_short_opt(char *opt, struct option *opts)
	_GETARG_FIND_OPT_TEMPLATE(opts, (opts[i].short_name == opt[1]))

#undef _GETARG_FIND_OPT_TEMPLATE

static enum GETARG_RESULT _getarg_apply_help_opt(const char *usages[]);

static enum GETARG_RESULT _getarg_err_opt_not_found(int line, const char *opt);
#define err_opt_not_found(STR) _getarg_err_opt_not_found(__LINE__, STR)

static enum GETARG_RESULT _getarg_parse_arg(int *argc,
		char **argv[],
		struct option *opt);

static enum GETARG_RESULT _getarg_parse_long_opt(int *argc,
		char **argv[],
		struct option *opts);

static enum GETARG_RESULT _getarg_parse_opt(int *argc,
		char **argv[],
		struct option *opts);

static enum GETARG_RESULT _getarg_parse_short_opt(int *argc,
		char **argv[],
		struct option *opts);

enum GETARG_RESULT
_getarg_apply_help_opt(const char *usages[])
{
	for (int i = 0; usages[i] != NULL; i++)
		puts(usages[i]);
	return GETARG_RESULT_APPLIED_HELP_OPT;
}

enum GETARG_RESULT
_getarg_err_opt_not_found(int line, const char *opt)
{
	fprintf(stderr, "[libgetarg:%s:%d]: option not found: '%s'\n",
			__FILE__,
			line,
			opt);
	return GETARG_RESULT_OPT_NOT_FOUND;
}

enum GETARG_RESULT
_getarg_parse_arg(int *argc, char **argv[], struct option *opt)
{
	char *arg = **argv, *tmp;
	switch (opt->type) {
	case GETARG_OPT_END:
		break;
	case GETARG_OPT_HELP:
		return _getarg_apply_help_opt(opt->value.v);
	case GETARG_OPT_FLAG:
		*(uint64_t*)opt->value.v |= opt->data;
		break;
	case GETARG_OPT_MANUAL:
		return opt->value.f(argc, argv, opt);
	case GETARG_OPT_STRING:
		*(char**)opt->value.v = arg;
		_GETARG_SHIFT(argc, argv);
		break;
	case GETARG_OPT_UINT:
		*((uint64_t*)opt->value.v) = strtoull(arg, &tmp, 10);
		if (tmp && tmp[0] != '\0')
			return GETARG_RESULT_PARSE_ARG_FAILED;
		break;
	}
	return GETARG_RESULT_SUCCESSFUL;
}

enum GETARG_RESULT
_getarg_parse_long_opt(int *argc, char **argv[], struct option *opts)
{
	struct option *opt;
	if ((**argv)[2] == '\0')
		return GETARG_RESULT_DOUBLE_DASH;

	if (!(opt = _getarg_find_long_opt(**argv, opts)))
		return err_opt_not_found(**argv);

	_GETARG_SHIFT(argc, argv);
	return _getarg_parse_arg(argc, argv, opt);
}

enum GETARG_RESULT
_getarg_parse_opt(int *argc, char **argv[], struct option *opts)
{
	if ((**argv)[1] == '-')
		return _getarg_parse_long_opt(argc, argv, opts);
	return _getarg_parse_short_opt(argc, argv, opts);
}

enum GETARG_RESULT
_getarg_parse_short_opt(int *argc, char **argv[], struct option *opts)
{
	struct option *opt;
	if ((**argv)[1] == '\0')
		return GETARG_RESULT_DASH;

	if (!(opt = _getarg_find_short_opt(**argv, opts)))
		return err_opt_not_found(**argv);

	if ((**argv)[2] != '\0') {
		(**argv) = &(**argv)[2];
	} else {
		_GETARG_SHIFT(argc, argv);
	}
	return _getarg_parse_arg(argc, argv, opt);
}

enum GETARG_RESULT
getarg(int *argc, char **argv[], struct option *opts)
{
	if (*argc <= 0)
		return GETARG_RESULT_END;
	if ((**argv)[0] == '-')
		return _getarg_parse_opt(argc, argv, opts);
	return GETARG_RESULT_UNKNOWN;
}
#endif /* GETARG_IMPL */
