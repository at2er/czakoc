#include <string.h>
#include "zako.h"

static struct zk_ident *find_ident_once(struct zk_scope *scope,
		const struct str *name);

struct zk_ident *
find_ident_once(struct zk_scope *scope, const struct str *name)
{
	for (int i = 0; i < scope->idents.n; i++) {
		if (name->len != strlen(scope->idents.e[i]->name))
			continue;
		if (strncmp(scope->idents.e[i]->name, name->s, name->len) == 0)
			return scope->idents.e[i];
	}
	return NULL;
}

struct zk_ident *
find_ident(struct zk_scope *scope, const struct str *name)
{
	struct zk_ident *id;
	while (scope) {
		id = find_ident_once(scope, name);
		if (id)
			return id;
		scope = scope->parent;
	}
	return NULL;
}

struct zk_ident *
find_struct_member(struct zk_struct_type *type, const struct str *name)
{
	for (int i = 0; i < type->members.n; i++) {
		if (name->len != strlen(type->members.e[i]->name))
			continue;
		if (strncmp(name->s, type->members.e[i]->name, name->len) == 0)
			return type->members.e[i];
	}
	return NULL;
}
