/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "czakoc.h"
#include "err.h"
#define GETARG_IMPL
#include "getarg.h"
#include "parser.h"

static int parse_cmdline_args(int argc, char *argv[]);

uint64_t czakoc_flags = 0;

static const char *entry_file = NULL;
static const char *usages[] = {
"Usage: czakoc [OPTIONS]... <FILE>",
"",
"      --output-ast:           output AST to stdout",
"      --output-lexer-tokens:  output tokens from lexer to stdout",
"  -h, --help:                 show usages",
NULL
};
static struct option options[] = {
	OPT_FLAG("output-ast", NO_SHORT_NAME,
			&czakoc_flags,
			CZAKOC_OUTPUT_AST),
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
