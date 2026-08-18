#include <stdbool.h>
#include <string.h>
#include "ealloc.h"
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

struct zk_type *
deref_type(struct zk_type *t)
{
	if (!t)
		return NULL;
	if (t->builtin == ZK_TYPE_REF)
		return deref_type(t->u.type);
	return t;
}

struct zk_type *
dup_type(struct zk_type *t)
{
	struct zk_type *res = ecalloc(1, sizeof(*t));
	memcpy(res, t, sizeof(*t));
	return res;
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

struct zk_type *
monomorphize(struct zk_type *instance, struct zk_type *t)
{
	switch (instance->builtin) {
	case ZK_ARR:
		monomorphize_arr(&instance->u.arr_type, t);
		break;
	case ZK_STRUCT:
		monomorphize_struct(&instance->u.struct_type, t);
		break;
	case ZK_PTR:
		instance->u.type = dup_type(instance->u.type);
		monomorphize(instance->u.type, t);
		break;
	case ZK_TYPE_REF:
		memcpy(instance, instance->u.type, sizeof(*instance));
		monomorphize(instance, t);
		break;
	case ZK_GENERIC_TYPE:
		instance->builtin = ZK_TYPE_REF;
		instance->u.type = t;
		break;
	default:
		break;
	}
	return instance;
}

struct zk_arr_type *
monomorphize_arr(struct zk_arr_type *instance, struct zk_type *t)
{
	instance->type = dup_type(instance->type);
	monomorphize(instance->type, t);
	return instance;
}

struct zk_struct_type *
monomorphize_struct(struct zk_struct_type *instance, struct zk_type *t)
{
	struct zk_ident *id;
	zk_idents_t members;

	instance->generics = false;
	darr_init(&instance->generic_instances);
	darr_init(&members);
	for (int i = 0; i < instance->members.n; i++) {
		id = ecalloc(1, sizeof(struct zk_ident));
		memcpy(id, instance->members.e[i], sizeof(*id));
		monomorphize(&id->type, t);
		darr_append(&members, id);
	}
	memcpy(&instance->members, &members, sizeof(members));
	return instance;
}

struct zk_type *
open_type(struct zk_type *type)
{
	if (!type)
		return NULL;
	switch (type->builtin) {
	case ZK_TYPE_REF:
		return open_type(type->u.type);
	case ZK_GENERIC_INSTANCE_TYPE:
		return open_type(type->u.generic_instance_type->instance);
	default:
		return type;
	}
}
