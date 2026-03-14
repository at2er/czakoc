/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <libgen.h>
#include <string.h>
#include <unistd.h>
#include "czakoc.h"
#include "err.h"
#include "module.h"
#include "parser/cache.h"
#include "parser/parser.h"

#define GETARG_IMPL
#include "getarg.h"

static int parse_cmdline_args(int argc, char *argv[]);

mode_t czakoc_file_mode;
uint64_t czakoc_flags = 0;
const char *czakoc_cache_dir = NULL;
const char *czakoc_cwd = NULL;
const char *czakoc_root_mod = NULL;

static const char *entry_file = NULL;
static const char *usages[] = {
"Usage: czakoc [OPTIONS]... <FILE>",
"",
"  -D, --cache-dir:            cache directory",
"  -M, --root-module:          root module's name",
"      --output-ast:           output AST to stdout",
"      --output-ir:            output libmcb's IR",
"      --output-lexer-tokens:  output tokens from lexer to stdout",
"  -h, --help:                 show usages",
NULL
};
static struct option options[] = {
	OPT_STRING("cache-dir",   'D', &czakoc_cache_dir),
	OPT_STRING("root-module", 'M', &czakoc_root_mod),
	OPT_FLAG("output-ast", NO_SHORT_NAME,
			&czakoc_flags,
			CZAKOC_OUTPUT_AST),
	OPT_FLAG("output-ir", NO_SHORT_NAME,
			&czakoc_flags,
			CZAKOC_OUTPUT_IR),
	OPT_FLAG("output-lexer-tokens", NO_SHORT_NAME,
			&czakoc_flags,
			CZAKOC_OUTPUT_LEXER_TOKENS),
	OPT_HELP("help", 'h', usages),
	OPT_END
};

int
parse_cmdline_args(int argc, char *argv[])
{
	enum GETARG_RESULT getarg_result;
	GETARG_BEGIN(getarg_result, argc, argv, options) {
	case GETARG_RESULT_SUCCESSFUL:
		break;
	case GETARG_RESULT_UNKNOWN:
		if (entry_file)
			return 1;
		entry_file = *argv;
		GETARG_SHIFT(argc, argv);
		break;
	default:
		return 1;
	} GETARG_END;
	return 0;
}

int
main(int argc, char *argv[])
{
	char *cwd = NULL;
	struct zako_module *mod;
	if (parse_cmdline_args(argc, argv))
		return 1;
	if (!entry_file) {
		print_err_msg("no input file");
		return 1;
	}

	czakoc_file_mode = 0777 & ~umask(0);

	cwd = realpath(entry_file, NULL);
	czakoc_cwd = dirname(cwd);
	if (!czakoc_root_mod)
		czakoc_root_mod = basename(cwd);

	if (create_cache_dir())
		return 1;

	mod = parse_file(entry_file);
	if (!mod)
		return 1;
	free_module(mod);

	if (cwd)
		free(cwd);

	return 0;
}
