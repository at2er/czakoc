/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "ident.h"
#include "type.h"
#include "utils.h"
#include "../ealloc.h"
#include "../ident.h"

struct zako_ident *
parse_ident_sign(struct sclexer_tok *tok, struct parser *parser)
{
	struct zako_ident *ident;
	assert(tok && parser);
	assert(tok->kind == SCLEXER_IDENT);
	ident = ecalloc(1, sizeof(*ident));
	ident->name = dup_slice_to_cstr(&tok->data.str);
	ident->type = parse_type(parser);
	if (!ident->type)
		goto err_parse_type;
	return ident;
err_parse_type:
	free(ident->name);
	free(ident);
	return NULL;
}
