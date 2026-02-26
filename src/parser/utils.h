/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_PARSER_UTILS_H
#define CZAKOC_PARSER_UTILS_H
#include "parser.h"
#include "sclexer.h"

char *dup_slice_to_cstr(struct sclexer_str_slice *slice);
struct sclexer_tok *eat_tok(struct parser *parser);
struct sclexer_tok *eat_tok_skip_white(struct parser *parser);
struct sclexer_tok *peek_tok(struct parser *parser);
struct sclexer_tok *peek_tok_to(struct parser *parser, int count);
struct sclexer_tok *peek_tok_skip_white(struct parser *parser);

#endif
