/***************************************************************************
    begin........: June 2012
    copyright....: Sebastian Fedrau
    email........: lord-kefir@arcor.de
 ***************************************************************************/

/***************************************************************************
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License v3 as published by
    the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License v3 for more details.
 ***************************************************************************/
/*!
 * \file slist.c
 * \brief A singly-linked list.
 * \author Sebastian Fedrau <lord-kefir@arcor.de>
 * \version 0.1.0
 * \date 27. June 2012
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "slist.h"

/*
 *	helpers:
 */
static inline SListItem *
_slist_item_new(Allocator *allocator, void *data)
{
	SListItem *item = NULL;

	if(allocator)
	{
		item = (SListItem *)allocator->alloc(allocator);
	}

	if(!item && !(item = (SListItem *)malloc(sizeof(SListItem))))
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		abort();
	}

	memset(item, 0, sizeof(SListItem));
	item->data = data;

	return item;
}

static SListItem *
_slist_find(SList *list, SListItem *offset, void const *data)
{
	SListItem *p;

	if(offset)
	{
		p = offset;
	}
	else
	{
		p = list->head;
	}

	while(p)
	{
		if(list->equals(p->data, data))
		{
			return p;
		}

		p = p->next;
	}

	return NULL;
}

static void
_slist_remove(SList *list, SListItem *prev, SListItem *item)
{
	if(prev)
	{
		prev->next = item->next;
	}
	else
	{
		list->head = item->next;
	}

	if(list->tail == item)
	{
		list->tail = prev;
	}

	/* free allocated memory */
	if(list->free)
	{
		list->free(item->data);
	}

	if(list->allocator)
	{
		list->allocator->free(list->allocator, item);
	}
	else
	{
		free(item);
	}

	--list->count;
}

/*
 *	public:
 */
SList *
slist_new(EqualFunc equals, FreeFunc free, Allocator *allocator)
{
	SList *list;

	if(!(list = (SList *)malloc(sizeof(SList))))
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		abort();
	}

	slist_init(list, equals, free, allocator);

	return list;
}

void
slist_init(SList *list, EqualFunc equals, FreeFunc free, Allocator *allocator)
{
	assert(equals != NULL);

	memset(list, 0, sizeof(SList));
	list->equals = equals;
	list->free = free;
	list->allocator = allocator;
}

void
slist_free(SList *list)
{
	SListItem *iter;
	SListItem *item;

	assert(list != NULL);

	iter = list->head;

	while(iter)
	{
		item = iter;
		iter = iter->next;

		if(list->free)
		{
			list->free(item->data);
		}

		if(list->allocator)
		{
			list->allocator->free(list->allocator, item);
		}
		else
		{
			free(item);
		}
	}
}

void
slist_destroy(SList *list)
{
	slist_free(list);
	free(list);
}

SListItem *
slist_append(SList *list, void *data)
{
	SListItem *item;

	assert(list != NULL);

	item = _slist_item_new(list->allocator, data);

	if(list->tail)
	{
		list->tail->next = item;
	}

	list->tail = item;

	if(!list->head)
	{
		list->head = item;
	}

	++list->count;

	return item;
}

SListItem *
slist_prepend(SList *list, void *data)
{
	SListItem *item;

	assert(list != NULL);

	item = _slist_item_new(list->allocator, data);

	if(list->head)
	{
		item->next = list->head;
	}

	list->head = item;

	if(!list->tail)
	{
		list->tail = item;
	}

	++list->count;

	return item;
}

SListItem *
slist_insert_sorted(SList *list, void *data, CompareFunc compare)
{
	SListItem *iter;
	SListItem *item;
	SListItem *prev = NULL;

	assert(list != NULL);
	assert(compare != NULL);

	if((iter = list->head))
	{
		/* test if new item can be appended */
		if(compare(list->tail->data, data) <= 0)
		{
			return slist_append(list, data);
		}
		else if(list->count == 1)
		{
			/* prepend item */
			return slist_prepend(list, data);
		}

		/* insert list item into list */
		item = _slist_item_new(list->allocator, data);

		while(iter)
		{
			if(compare(iter->data, data) >= 0)
			{
				if(prev)
				{
					prev->next = item;
				}
				else
				{
					list->head = item;
				}

				item->next = iter;
				break;
			}

			prev = iter;
			iter = iter->next;
		}
	}
	else
	{
		/* insert head */
		item = _slist_item_new(list->allocator, data);
		list->head = item;
		list->tail = item;
	}

	++list->count;

	return item;
}

void
slist_remove(SList *list, SListItem *item)
{
	SListItem *p;
	SListItem *prev = NULL;

	assert(list != NULL);
	assert(list->head != NULL);

	p = list->head;

	while(p)
	{
		if(p == item)
		{
			/* remove list item */
			_slist_remove(list, prev, p);
			break;
		}

		prev = p;
		p = p->next;
	}
}

void
slist_remove_by_data(SList *list, void *data, bool remove_all)
{
	SListItem *iter;
	SListItem *next;
	SListItem *prev = NULL;

	assert(list != NULL);
	assert(list->equals != NULL);

	iter = list->head;

	while(iter)
	{
		if(list->equals(iter->data, data))
		{
			next = iter->next;
			_slist_remove(list, prev, iter);
			iter = next;

			/* leave loop if we don't have to remove all desired items */
			if(!remove_all)
			{
				break;
			}
		}
		else
		{
			prev = iter;
			iter = iter->next;
		}
	}

	if(list->count == 1)
	{
		list->tail = list->head;
	}
}

void *
slist_pop(SList *list)
{
	SListItem *item;
	void *data = NULL;

	assert(list != NULL);

	if((item = list->head))
	{
		if(!(list->head = list->head->next))
		{
			list->tail = NULL;
		}

		data = item->data;

		if(list->allocator)
		{
			list->allocator->free(list->allocator, item);
		}
		else
		{
			free(item);
		}

		--list->count;
	}

	return data;
}

bool
slist_contains(SList *list, void *data)
{
	assert(list != NULL);
	assert(list->equals != NULL);

	return _slist_find(list, NULL, data) ? true : false;
}

void
slist_clear(SList *list)
{
	SListItem *iter;
	SListItem *next;

	assert(list != NULL);

	iter = list->head;

	while(iter)
	{
		next = iter->next;

		if(list->free)
		{
			list->free(iter->data);
		}

		if(list->allocator)
		{
			list->allocator->free(list->allocator, iter);
		}
		else
		{
			free(iter);
		}

		iter = next;
	}

	list->count = 0;
	list->head = list->tail = NULL;
}

SListItem *
slist_find(SList *list, SListItem *offset, void const *data)
{
	return _slist_find(list, offset, data);
}

inline size_t
slist_count(SList *list)
{
	assert(list != NULL);

	return list->count;
}

inline SListItem *
slist_head(SList *list)
{
	assert(list != NULL);

	return list->head;
}

inline bool
slist_empty(SList *list)
{
	assert(list != NULL);

	return list->head ? true : false;
}

void
slist_reorder(SList *list, SListItem *item, CompareFunc compare)
{
	SListItem *iter;
	SListItem *prev = NULL;
	bool inserted = false;

	assert(list != NULL);
	assert(item != NULL);

	/* detach list item */
	iter = list->head;

	while(iter)
	{
		if(iter == item)
		{
			if(prev)
			{
				prev->next = iter->next;
			}
			else
			{
				list->head = iter->next;
			}

			if(list->tail == item)
			{
				list->tail = prev;
			}

			break;
		}

		prev = iter;
		iter = iter->next;
	}

	/* insert list item */
	iter = list->head;
	prev = NULL;

	while(iter)
	{
		if(compare(iter->data, item->data) >= 0)
		{
			if(prev)
			{
				prev->next = item;
			}
			else
			{
				list->head = item;
			}

			item->next = iter;
			inserted = true;
			break;
		}

		prev = iter;
		iter = iter->next;
	}

	if(!inserted)
	{
		list->tail->next = item;
		list->tail = item;
		item->next = NULL;
	}
}

