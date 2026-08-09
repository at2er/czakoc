#include <stdio.h>
#include "die.h"
#include "parser.h"
#include "zako.h"

int
main(int argc, char *argv[])
{
	struct zk_mod *root;
	if (argc <= 1)
		die("miss file name\n");

	root = parse(argv[1]);
	if (!root)
		return 1;
	return 0;
}
