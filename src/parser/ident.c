/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include "ident.h"
#include "type.h"
#include "utils.h"
#include "../ealloc.h"
#include "../ident.h"
#include "../str.h"

static char *get_mcb_name(const char *name, struct parser *parser);

char *
get_mcb_name(const char *name, struct parser *parser)
{
	struct str s;
	assert(name && parser);
	estr_from_cstr(&s, parser->mod->prefix);
	estr_append_cstr(&s, name);
	return s.s;
}

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
	ident->mcb_name = get_mcb_name(ident->name, parser);
	return ident;
err_parse_type:
	free(ident->name);
	free(ident);
	return NULL;
}
