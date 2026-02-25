/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <stdlib.h>
#include "fn.h"
#include "parser.h"
#include "type.h"
#include "utils.h"
#include "../ealloc.h"
#include "../err.h"
#include "../lexer.h"
#include "../type.h"

struct zako_type *
parse_type(struct parser *parser)
{
	struct sclexer_tok *tok;
	struct zako_type *type;
	assert(parser);
	tok = eat_tok(parser);
	if (tok->kind != SCLEXER_KEYWORD)
		goto err_unexpected_token;
	type = ecalloc(1, sizeof(*type));
	switch (tok->data.keyword) {
	case KEYWORD_FN:
		type->builtin = FN_TYPE;
		if (parse_fn_sign(&type->inner.fn, parser))
			goto err_free_type;
		break;
	case KEYWORD_I8:  type->builtin = I8_TYPE;  break;
	case KEYWORD_I16: type->builtin = I16_TYPE; break;
	case KEYWORD_I32: type->builtin = I32_TYPE; break;
	case KEYWORD_I64: type->builtin = I64_TYPE; break;
	case KEYWORD_U8:  type->builtin = U8_TYPE;  break;
	case KEYWORD_U16: type->builtin = U16_TYPE; break;
	case KEYWORD_U32: type->builtin = U32_TYPE; break;
	case KEYWORD_U64: type->builtin = U64_TYPE; break;
	default: goto err_unexpected_keyword; break;
	}
	return type;
err_unexpected_token:
	print_err("unexpected token", tok);
	return NULL;
err_unexpected_keyword:
	printf_err("unexpected keyword '%s'",
			tok,
			cstr_keyword(tok->data.keyword));
err_free_type:
	free(type);
	return NULL;
}
