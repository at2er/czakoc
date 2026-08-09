/* sex! sex! sex! */
#ifndef CZAKOC_CSEX_H
#define CZAKOC_CSEX_H
#include <stdio.h>
#include "parser.h"
#include "zako.h"

struct codegen {
	int blk_lv;
	FILE *out;

	zk_top_stmts_t stmts;

	char *name_prefix;

	unsigned int after_arr_type:1;
	unsigned int arr_siz;
};

void codegen(struct codegen *cg, FILE *out);

#endif
