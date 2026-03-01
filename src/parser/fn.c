/* SPDX-License-Identifier: GPL-3.0-or-later */
#include "fn.h"
#include "parser.h"
#include "sclexer.h"
#include "scope.h"
#include "stmt.h"
#include "ident.h"
#include "type.h"
#include "utils.h"
#include "value.h"
#include "../darr.h"
#include "../ealloc.h"
#include "../err.h"
#include "../fn.h"
#include "../ident.h"
#include "../lexer.h"

static struct zako_fn_definition *parse_fn_body(
		struct zako_fn_declaration *declaration,
		struct parser *parser);
static struct zako_fn_declaration *parse_fn_declaration(
		struct sclexer_tok *tok,
		struct parser *parser,
		bool public);

struct zako_fn_definition *
parse_fn_body(struct zako_fn_declaration *declaration, struct parser *parser)
{
	struct zako_fn_definition *definition;
	struct zako_stmt *stmt;
	struct sclexer_tok *tok;
	assert(parser);

	definition = ecalloc(1, sizeof(*definition));
	definition->declaration = declaration;

	parser->cur_scope->fn = definition;

	tok = eat_tok_skip_white(parser);

	if (tok->kind != SCLEXER_INDENT_BLOCK_BEGIN)
		goto err_unexpected_token;
	tok = eat_tok_skip_white(parser);
	while (tok->kind != SCLEXER_INDENT_BLOCK_END) {
		stmt = parse_stmt(tok, parser);
		if (!stmt)
			goto err_free_definition;
		darr_append(definition->stmts, definition->stmts_count, stmt);
		tok = eat_tok_skip_white(parser);
	}

	return definition;
err_unexpected_token:
	print_err("unexpected token", tok);
err_free_definition:
	definition->declaration = NULL;
	free_fn_definition(definition);
	return NULL;
}

struct zako_fn_declaration *
parse_fn_declaration(
		struct sclexer_tok *tok,
		struct parser *parser,
		bool public)
{
	struct zako_fn_declaration *declaration;
	struct zako_ident *ident;
	assert(tok && parser);
	assert(tok->kind == SCLEXER_IDENT);
	ident = parse_ident_sign(tok, parser);
	if (!ident)
		return NULL;
	declaration = ecalloc(1, sizeof(*declaration));
	declaration->ident = ident;
	declaration->public = public;
	darr_append(parser->cur_scope->parent->idents,
			parser->cur_scope->parent->idents_count,
			ident);
	return declaration;
}

struct zako_fn_call *
parse_fn_call(struct zako_ident *callee,
		struct parser *parser)
{
	struct zako_fn_call *call;
	int i = 0;
	struct zako_value *value;
	assert(callee && parser);
	call = ecalloc(1, sizeof(*call));
	call->fn = callee;
	for (; i < callee->type->inner.fn.argc; i++) {
		value = parse_value(parser);
		if (!value)
			goto err_parse_value;
		darr_append(call->args, call->argc, value);
	}
	return call;
err_parse_value:
	if (i < callee->type->inner.fn.argc) {
		printf_err_cont("call '%s': too few arguments", callee->name);
	} else {
		printf_err_cont("call '%s': too many arguments", callee->name);
	}
	free_fn_call(call);
	return NULL;
}

struct zako_toplevel_stmt *
parse_fn_definition(struct sclexer_tok *tok, struct parser *parser, bool public)
{
	struct zako_fn_declaration *declaration;
	struct zako_toplevel_stmt *stmt;
	assert(tok && parser);

	enter_scope(parser);

	declaration = parse_fn_declaration(tok, parser, public);
	if (!declaration)
		return NULL;
	stmt = ecalloc(1, sizeof(*stmt));
	stmt->kind = FN_DECLARATION;
	stmt->inner.fn_declaration = declaration;
	tok = peek_tok(parser);
	if (tok->kind != SCLEXER_SYMBOL || tok->data.symbol != SYM_ASSIGN) {
		if (tok->kind != SCLEXER_EOL && tok->kind != SCLEXER_EOF)
			goto err_unexpected_token;
		exit_scope(parser);
		return stmt; /* declaration */
	}
	eat_tok(parser);
	stmt->inner.fn_definition = parse_fn_body(declaration, parser);
	if (!stmt->inner.fn_definition)
		goto err_parse_fn_body;
	stmt->kind = FN_DEFINITION;

	exit_scope(parser);

	return stmt;
err_unexpected_token:
	print_err("unexpected token", tok);
err_parse_fn_body:
	exit_scope(parser);
	stmt->inner.fn_declaration = declaration;
	free_toplevel_stmt(stmt);
	return NULL;
}

int
parse_fn_sign(struct zako_fn_type *type, struct parser *parser)
{
	struct zako_ident *arg;
	struct sclexer_tok *tok;
	assert(type && parser);
	tok = peek_tok(parser);
	if (tok->kind == SCLEXER_SYMBOL) {
		if (tok->data.symbol != SYM_PAREN_L)
			goto err_unexpected_symbol;
		eat_tok(parser);
	} else if (tok->kind == SCLEXER_KEYWORD) {
		type->type = parse_type(parser);
		return 0;
	} else {
		goto err_unexpected_token;
	}
	tok = eat_tok(parser);
	arg = parse_ident_sign(tok, parser);
	if (!arg)
		return 1;
	darr_append(type->args, type->argc, arg);
	tok = peek_tok(parser);
	while (tok->kind == SCLEXER_SYMBOL && tok->data.symbol == SYM_COMMA) {
		eat_tok(parser);
		tok = eat_tok(parser);
		arg = parse_ident_sign(tok, parser);
		if (!arg)
			goto err_free_args;
		darr_append(type->args, type->argc, arg);
		tok = peek_tok(parser);
	}
	if (tok->kind != SCLEXER_SYMBOL)
		goto err_unexpected_token;
	if (tok->data.symbol != SYM_PAREN_R)
		goto err_unexpected_symbol;
	eat_tok(parser);
	tok = peek_tok(parser);
	type->type = parse_type(parser);
	if (!type->type)
		goto err_free_args;
	for (int i = 0; i < type->argc; i++) {
		darr_append(parser->cur_scope->idents,
				parser->cur_scope->idents_count,
				type->args[i]);
	}
	return 0;
err_unexpected_symbol:
	printf_err("unexpected symbol '%s'",
			tok,
			cstr_symbol(tok->data.symbol));
	return 1;
err_unexpected_token:
	print_err("unexpected token", tok);
	return 1;
err_free_args:
	destroy_fn_type(type);
	return 1;
}
