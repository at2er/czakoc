/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdio.h>
#include <string.h>

#define SCLEXER_IMPL
#include "sclexer.h"

enum TOKEN {
	TOK_ASSIGN,
	TOK_DOT,
	TOK_EQEQ,
	TOK_EXPECT,
	TOK_IF,
	TOK_RETURN,
	TOK_SCOPE,

	TOK_LBRACE,
	TOK_RBRACE,
	TOK_LPAREN,
	TOK_RPAREN,

	TOK_IDENT,

	TOK_ERR
};

static const char *tokens[] = {
	[TOK_ASSIGN] = "=",
	[TOK_DOT]    = ".",
	[TOK_EQEQ]   = "==",
	[TOK_EXPECT] = "expect",
	[TOK_IF]     = "if",
	[TOK_RETURN] = "return",
	[TOK_SCOPE]  = "scope",
	[TOK_LBRACE] = "{",
	[TOK_RBRACE] = "}",
	[TOK_LPAREN] = "(",
	[TOK_RPAREN] = ")",
	NULL
};
static const char *comments[] = {
	"--",
	NULL
};

int
main()
{
	struct sclexer lexer = {0};
	char *src;
	struct sclexer_tok tok;

	sclexer_read_file(stdin, &src);

	lexer.tokens = tokens;
	lexer.comments = comments;
	lexer.ident_tok = TOK_IDENT;
	lexer.int_tok = -1;
	sclexer_init(&lexer, src);

	while (sclexer_next(&lexer, &tok)) {
		printf("%d:%d\t%d\t\"%.*s\"\n", tok.row, tok.col, tok.type, tok.len, tok.str);
	}

	return 0;
}
