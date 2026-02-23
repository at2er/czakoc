/* SPDX-License-Identifier: GPL-3.0-or-later */
// #include "compiler.h"
#include "err.h"
#define GETARG_IMPL
#include "getarg.h"
#include "parser.h"

static int parse_cmdline_args(int argc, char *argv[]);

static const char *entry_file = NULL;
static const char *usages[] = {
"Usage: czakoc [OPTIONS]... <FILE>",
NULL
};
static struct option options[] = {
	OPT_HELP("--help", 'h', usages),
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
	if (parse_cmdline_args(argc, argv))
		return 1;
	if (!entry_file) {
		print_err_msg("no input file");
		return 1;
	}
	if (!parse_file(entry_file))
		return 1;
	return 0;
}
