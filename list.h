/* Thanks for wayland-util[1]
 *
 * Copyright © 2008-2011 Kristian Høgsberg
 * Copyright © 2011 Intel Corporation
 * Copyright © 2026 at2er
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * [1] wayland-util:
 *     https://cgit.freedesktop.org/wayland/wayland/tree/src/wayland-util.c
 * */
#ifndef UTILSH_LIST_H
#define UTILSH_LIST_H
#include <stddef.h>

struct utilsh_list {
	struct utilsh_list *prv;
	struct utilsh_list *nex;
};

struct utilsh_list_head {
	struct utilsh_list *beg;
	struct utilsh_list *end;
};

#define utilsh_list_container_of(PTR, STRUCT, MEMBER) \
	((PTR) ? (STRUCT*)((char*)(PTR) - offsetof(STRUCT, MEMBER)) : NULL)

#define _utilsh_list_for_each_iter(CUR, TYPE, LINK) \
	(CUR ? utilsh_list_container_of(CUR->LINK.nex, TYPE, LINK) : NULL)

#define _utilsh_list_for_each_iter_prv(CUR, TYPE, LINK) \
	(CUR ? utilsh_list_container_of(CUR->LINK.prv, TYPE, LINK) : NULL)

#define _utilsh_list_for_each(TYPE, CUR, BEG, NEX, LINK, ITER) \
	for (TYPE *CUR = utilsh_list_container_of((BEG), TYPE, LINK), \
			*NEX = ITER(CUR, TYPE, LINK); \
			CUR != NULL; \
			CUR = NEX, \
			NEX = ITER(CUR, TYPE, LINK))

#define utilsh_list_for_each(TYPE, CUR, BEG, NEX, LINK) \
	_utilsh_list_for_each(TYPE, CUR, BEG, NEX, LINK, \
			_utilsh_list_for_each_iter)

#define utilsh_list_for_each_prv(TYPE, CUR, BEG, NEX, LINK) \
	_utilsh_list_for_each(TYPE, CUR, BEG, NEX, LINK, \
			_utilsh_list_for_each_iter_prv)

#define utilsh_list_for_each_unsafe(TYPE, CUR, BEG, NEX, LINK) \
	for (TYPE *CUR = utilsh_list_container_of((BEG), TYPE, LINK); \
			CUR != NULL; \
			CUR = _utilsh_list_for_each_iter(CUR, TYPE, LINK))

void utilsh_list_init(struct utilsh_list_head *list);
void utilsh_list_insert(
		struct utilsh_list_head *list,
		struct utilsh_list *prv,
		struct utilsh_list *elem);
void utilsh_list_remove(
		struct utilsh_list_head *list,
		struct utilsh_list *elem);

#ifdef UTILSH_LIST_STRIP
#define list_container_of    utilsh_list_container_of
#define list_for_each        utilsh_list_for_each
#define list_for_each_prv    utilsh_list_for_each_prv
#define list_for_each_unsafe utilsh_list_for_each_unsafe
#define list_init            utilsh_list_init
#define list_insert          utilsh_list_insert
#define list_remove          utilsh_list_remove
#endif

#endif

#ifdef UTILSH_LIST_IMPL
#include <assert.h>

void
utilsh_list_init(struct utilsh_list_head *list)
{
	list->beg = list->end = NULL;
}

void
utilsh_list_insert(
		struct utilsh_list_head *list,
		struct utilsh_list *prv,
		struct utilsh_list *elem)
{
	elem->prv = prv;
	if (prv) {
		elem->nex = prv->nex;
		if (elem->nex)
			elem->nex->prv = elem;
		prv->nex = elem;
	} else {
		elem->nex = list->beg;
		if (list->beg)
			list->beg->prv = elem;
		list->beg = elem;
	}
	if (prv == list->end)
		list->end = elem;
}

void
utilsh_list_remove(
		struct utilsh_list_head *list,
		struct utilsh_list *elem)
{
	if (list->beg == elem)
		list->beg = elem->nex;
	if (list->end == elem)
		list->end = elem->prv;
	if (elem->prv)
		elem->prv->nex = elem->nex;
	if (elem->nex)
		elem->nex->prv = elem->prv;
}

#endif /* UTILSH_LIST_IMPL */
