/***************************************************************************
    begin........: June 2012
    copyright....: Sebastian Fedrau
    email........: sebastian.fedrau@gmail.com
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
/**
 * \file list.c
 * \brief A singly-linked list.
 * \author Sebastian Fedrau <sebastian.fedrau@gmail.com>
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "list.h"

/*
 *	helpers:
 */
static inline ListItem *
_list_item_new(Pool *pool, void *data)
{
	ListItem *item = NULL;

	if(pool)
	{
		item = (ListItem *)pool->alloc(pool);
	}

	if(!item && !(item = (ListItem *)malloc(sizeof(ListItem))))
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		abort();
	}

	memset(item, 0, sizeof(ListItem));
	item->data = data;

	return item;
}

static void
_list_detach(List *list, ListItem *item)
{
	assert(list != NULL);
	assert(item != NULL);

	/* check if there's a previous list item */
	if(item->prev)
	{
		/* update previous list item */
		item->prev->next = item->next;

		if(item->next)
		{
			item->next->prev = item->prev;
		}
	}
	else
	{
		/* update list item */
		if((list->head = item->next))
		{
			list->head->prev = NULL;
		}
	}

	/* update tail of list */
	if(item == list->tail)
	{
		list->tail = item->prev;
	}
}

static ListItem *
_list_find(const List *list, ListItem *offset, void const *data)
{
	ListItem *begin;
	ListItem *end;

	assert(list != NULL);

	if(offset)
	{
		begin = offset->next;
	}
	else
	{
		begin = list->head;
	}

	end = list->tail;

	while(begin)
	{
		if(!list->compare(begin->data, data))
		{
			return begin;
		}

		if(!list->compare(end->data, data))
		{
			return end;
		}

		begin = begin->next;

		if(begin == end)
		{
			return NULL;
		}

		end = end->prev;
	}

	return NULL;
}

/*
 *	public:
 */
List *
list_new(CompareFunc compare, FreeFunc free, Pool *pool)
{
	List *list;

	if(!(list = (List *)malloc(sizeof(List))))
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		abort();
	}

	list_init(list, compare, free, pool);

	return list;
}

void
list_init(List *list, CompareFunc compare, FreeFunc free, Pool *pool)
{
	assert(compare != NULL);

	memset(list, 0, sizeof(List));
	list->compare = compare;
	list->free = free;
	list->pool = pool;
}

void
list_free(List *list)
{
	ListItem *iter;

	assert(list != NULL);

	iter = list->head;

	while(iter)
	{
		ListItem *item = iter;

		iter = iter->next;

		if(list->free)
		{
			list->free(item->data);
		}

		if(list->pool)
		{
			list->pool->free(list->pool, item);
		}
		else
		{
			free(item);
		}
	}
}

void
list_destroy(List *list)
{
	assert(list != NULL);

	list_free(list);
	free(list);
}

ListItem *
list_append(List *list, void *data)
{
	ListItem *item;

	assert(list != NULL);

	item = _list_item_new(list->pool, data);

	if(list->tail)
	{
		list->tail->next = item;
		item->prev = list->tail;
	}

	list->tail = item;

	if(!list->head)
	{
		list->head = item;
	}

	++list->count;

	return item;
}

ListItem *
list_prepend(List *list, void *data)
{
	ListItem *item;

	assert(list != NULL);

	item = _list_item_new(list->pool, data);

	if(list->head)
	{
		item->next = list->head;
		list->head->prev = item;
	}

	list->head = item;

	if(!list->tail)
	{
		list->tail = item;
	}

	++list->count;

	return item;
}

ListItem *
list_insert_sorted(List *list, void *data)
{
	ListItem *iter;
	ListItem *item;

	assert(list != NULL);
	assert(list->compare != NULL);

	if((iter = list->head))
	{
		/* test if new item can be appended */
		if(list->compare(list->tail->data, data) <= 0)
		{
			return list_append(list, data);
		}
		else if(list->count == 1)
		{
			/* prepend item */
			return list_prepend(list, data);
		}

		/* insert list item into list */
		item = _list_item_new(list->pool, data);

		while(iter)
		{
			if(list->compare(iter->data, data) >= 0)
			{
				if(iter->prev)
				{
					iter->prev->next = item;
				}
				else
				{
					list->head = item;
				}

				item->next = iter;
				item->prev = iter->prev;
				iter->prev = item;
				break;
			}

			iter = iter->next;
		}
	}
	else
	{
		/* insert head */
		item = _list_item_new(list->pool, data);
		list->head = item;
		list->tail = item;
	}

	++list->count;

	return item;
}

void
list_remove(List *list, ListItem *item)
{
	ListItem *p;

	assert(list != NULL);
	assert(list->head != NULL);
	assert(item != NULL);

	p = item;

	/* detach list item */
	_list_detach(list, item);

	/* free allocated memory */
	if(list->free)
	{
		list->free(p->data);
	}

	if(list->pool)
	{
		list->pool->free(list->pool, p);
	}
	else
	{
		free(p);
	}

	--list->count;
}

void
list_remove_by_data(List *list, void *data, bool remove_all)
{
	ListItem *iter;
	ListItem *next;

	assert(list != NULL);
	assert(list->compare != NULL);

	iter = list->head;

	while(iter)
	{
		if(!list->compare(iter->data, data))
		{
			next = iter->next;
			list_remove(list, iter);
			iter = next;

			/* leave loop if we don't have to remove all desired items */
			if(!remove_all)
			{
				break;
			}
		}
		else
		{
			iter = iter->next;
		}
	}

	if(list->count == 1)
	{
		list->tail = list->head;
	}
}

void *
list_pop(List *list)
{
	ListItem *item;
	void *data = NULL;

	assert(list != NULL);

	if((item = list->head))
	{
		if((list->head = item->next))
		{
			list->head->prev = NULL;
		}
		else
		{
			list->head = NULL;
			list->tail = NULL;
		}

		data = item->data;

		if(list->pool)
		{
			list->pool->free(list->pool, item);
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
list_contains(const List *list, void *data)
{
	assert(list != NULL);
	assert(list->compare != NULL);

	return _list_find(list, NULL, data) ? true : false;
}

void
list_clear(List *list)
{
	ListItem *iter;
	ListItem *next;

	assert(list != NULL);

	iter = list->head;

	while(iter)
	{
		next = iter->next;

		if(list->free)
		{
			list->free(iter->data);
		}

		if(list->pool)
		{
			list->pool->free(list->pool, iter);
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

ListItem *
list_find(const List *list, ListItem *offset, void const *data)
{
	return _list_find(list, offset, data);
}

inline ListItem *
list_head(const List *list)
{
	assert(list != NULL);

	return list->head;
}

inline ListItem *
list_tail(const List *list)
{
	assert(list != NULL);

	return list->tail;
}

inline size_t
list_count(const List *list)
{
	assert(list != NULL);

	return list->count;
}

inline bool
list_empty(const List *list)
{
	assert(list != NULL);

	return list->head ? false : true;
}

void
list_item_free_data(const List *list, ListItem *item)
{
	assert(list != NULL);
	assert(item != NULL);

	if(list->free)
	{
		list->free(item->data);
	}

	item->data = NULL;
}

void
list_reorder(List *list, ListItem *item)
{
	ListItem *iter;
	bool inserted = false;

	assert(list != NULL);
	assert(list->compare != NULL);
	assert(item != NULL);

	/* detach list item */
	_list_detach(list, item);

	/* insert list item */
	iter = list->head;

	while(iter)
	{
		if(list->compare(iter->data, item->data) >= 0)
		{
			if(iter->prev)
			{
				item->prev = iter->prev;
				iter->prev->next = item;
			}
			else
			{
				list->head = item;
				item->prev = NULL;
			}

			item->next = iter;
			inserted = true;
			break;
		}

		iter = iter->next;
	}

	if(!inserted)
	{
		list->tail->next = item;
		item->prev = list->tail;
		list->tail = item;
		item->next = NULL;
	}
}

