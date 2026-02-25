/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include <string.h>
#include "parser.h"
#include "sclexer.h"
#include "utils.h"
#include "../ealloc.h"

char *
dup_slice_to_cstr(struct sclexer_str_slice *slice)
{
	char *res;
	res = ecalloc(slice->len + 1, sizeof(char));
	res[slice->len] = '\0';
	strncpy(res, slice->begin, slice->len);
	return res;
}

struct sclexer_tok *
eat_tok(struct parser *parser)
{
	assert(parser);
	if (parser->cur_tok >= parser->tokens_count)
		return NULL;
	parser->cur_tok++;
	assert(parser->tokens);
	return &parser->tokens[parser->cur_tok - 1];
}

struct sclexer_tok *
eat_tok_skip_white(struct parser *parser)
{
	struct sclexer_tok *tok;
	assert(parser);
	tok = eat_tok(parser);
	if (!tok)
		return NULL;
	while (tok->kind == SCLEXER_EOL) {
		tok = eat_tok(parser);
		if (!tok)
			return NULL;
	}
	return tok;
}

struct sclexer_tok *
peek_tok(struct parser *parser)
{
	assert(parser);
	if (parser->cur_tok >= parser->tokens_count)
		return NULL;
	assert(parser->tokens);
	return &parser->tokens[parser->cur_tok];
}
