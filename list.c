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
 * \brief Singly-linked list.
 * \author Sebastian Fedrau <sebastian.fedrau@gmail.com>
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "list.h"

List *
list_new(CompareFunc compare, FreeFunc free, Pool *pool)
{
	assert(compare != NULL);

	List *list = (List *)malloc(sizeof(List));

	if(!list)
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
	assert(list != NULL);
	assert(compare != NULL);

	memset(list, 0, sizeof(List));
	list->compare = compare;
	list->free = free;
	list->pool = pool;
}

void
list_free(List *list)
{
	assert(list != NULL);

	ListItem *iter = list->head;

	while(iter)
	{
		ListItem *item = iter;

		iter = iter->next;

		if(list->free && item->data)
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

static ListItem *
_list_item_new(Pool *pool, void *data)
{
	ListItem *item = NULL;

	if(pool)
	{
		item = (ListItem *)pool->alloc(pool);
	}
	else
	{
		item = (ListItem *)malloc(sizeof(ListItem));

		if(!item)
		{
			fprintf(stderr, "Couldn't allocate memory.\n");
			abort();
		}
	}

	memset(item, 0, sizeof(ListItem));
	item->data = data;

	return item;
}

ListItem *
list_append(List *list, void *data)
{
	assert(list != NULL);
	assert(list->count != SIZE_MAX);

	ListItem *item = _list_item_new(list->pool, data);

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
	assert(list != NULL);
	assert(list->count != SIZE_MAX);

	ListItem *item = _list_item_new(list->pool, data);

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
	assert(list != NULL);
	assert(list->compare != NULL);
	assert(list->count != SIZE_MAX);

	ListItem *iter = list->head;
	ListItem *item;

	if(iter)
	{
		if(list->compare(list->tail->data, data) <= 0)
		{
			item = list_append(list, data);
		}
		else if(list->count == 1)
		{
			item = list_prepend(list, data);
		}
		else
		{
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
	}
	else
	{
		item = _list_item_new(list->pool, data);
		list->head = item;
		list->tail = item;
	}

	++list->count;

	return item;
}

static void
_list_detach(List *list, ListItem *item)
{
	assert(list != NULL);
	assert(item != NULL);

	if(item->prev)
	{
		item->prev->next = item->next;

		if(item->next)
		{
			item->next->prev = item->prev;
		}
	}
	else
	{
		list->head = item->next;

		if(list->head)
		{
			list->head->prev = NULL;
		}
	}

	if(item == list->tail)
	{
		list->tail = item->prev;
	}
}
void
list_remove(List *list, ListItem *item)
{
	assert(list != NULL);
	assert(list->head != NULL);
	assert(item != NULL);

	ListItem *p = item;

	_list_detach(list, item);

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
	assert(list != NULL);
	assert(list->compare != NULL);

	ListItem *iter = list->head;
	ListItem *next;

	while(iter)
	{
		if(!list->compare(iter->data, data))
		{
			next = iter->next;
			list_remove(list, iter);
			iter = next;

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
	assert(list != NULL);

	ListItem *item = list->head;
	void *data = NULL;

	if(item)
	{
		list->head = item->next;

		if(list->head)
		{
			list->head->prev = NULL;
		}
		else
		{
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

static ListItem *
_list_find(const List *list, ListItem *offset, void const *data)
{
	assert(list != NULL);

	ListItem *begin = offset ? offset->next : list->head;
	ListItem *end = list->tail;

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

bool
list_contains(const List *list, void *data)
{
	assert(list != NULL);
	assert(list->compare != NULL);

	return _list_find(list, NULL, data) != NULL;
}

void
list_clear(List *list)
{
	assert(list != NULL);

	ListItem *iter = list->head;

	while(iter)
	{
		ListItem *next = iter->next;

		if(list->free && iter->data)
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
	assert(list != NULL);

	return _list_find(list, offset, data);
}

ListItem *
list_head(const List *list)
{
	assert(list != NULL);

	return list->head;
}

ListItem *
list_tail(const List *list)
{
	assert(list != NULL);

	return list->tail;
}

size_t
list_count(const List *list)
{
	assert(list != NULL);

	return list->count;
}

bool
list_empty(const List *list)
{
	assert(list != NULL);

	return list->head == NULL;
}

void
list_item_free_data(const List *list, ListItem *item)
{
	assert(list != NULL);
	assert(item != NULL);

	if(list->free && item->data)
	{
		list->free(item->data);
	}

	item->data = NULL;
}

void
list_reorder(List *list, ListItem *item)
{
	assert(list != NULL);
	assert(list->compare != NULL);
	assert(item != NULL);

	_list_detach(list, item);

	ListItem *iter = list->head;
	bool inserted = false;

	while(iter && !inserted)
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

