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

	unsigned int after_arr_type:1;
	unsigned int arr_siz;

	struct zk_type *generic_instance;
};

void codegen(struct codegen *cg, FILE *out);
char *codegen_get_realname(const char *prefix, const char *name);

#endif
