/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_LEXER_H
#define CZAKOC_LEXER_H
#include "parser/sclexer.h"

enum ZAKO_KEYWORD {
	KEYWORD_FN,
	KEYWORD_IF,
	KEYWORD_LET,
	KEYWORD_PUB,
	KEYWORD_RETURN,
	KEYWORD_THEN,

	KEYWORD_I8, KEYWORD_I16, KEYWORD_I32, KEYWORD_I64,
	KEYWORD_U8, KEYWORD_U16, KEYWORD_U32, KEYWORD_U64
};

enum ZAKO_SYMBOL {
	SYM_PAREN_L,
	SYM_PAREN_R,
	SYM_ASSIGN,
	SYM_COMMA,

	SYM_INFIX_LESS_EQUAL,

	SYM_INFIX_ADD,
	SYM_INFIX_DIV,
	SYM_INFIX_MUL,
	SYM_INFIX_SUB
};

const char *cstr_keyword(enum ZAKO_KEYWORD kw);
const char *cstr_symbol(enum ZAKO_SYMBOL sym);
char *init_lexer(struct sclexer *lexer, const char *path);

#endif
