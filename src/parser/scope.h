/* SPDX-License-Identifier: GPL-3.0-or-later */
#ifndef CZAKOC_PARSER_SCOPE_H
#define CZAKOC_PARSER_SCOPE_H

struct parser;

void enter_scope(struct parser *parser);
void exit_scope(struct parser *parser);

#endif
