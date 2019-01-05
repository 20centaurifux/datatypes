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
 * \brief Doubly-linked list.
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

static void
_list_item_free(List *list, ListItem *item)
{
	assert(list != NULL);
	assert(item != NULL);

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

void
list_free(List *list)
{
	assert(list != NULL);

	ListItem *iter = list->head;

	while(iter)
	{
		ListItem *item = iter;

		iter = iter->next;

		_list_item_free(list, item);
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

	ListItem *item = NULL;
	
	if(list->count < SIZE_MAX)
	{
		item = _list_item_new(list->pool, data);

		if(list->head)
		{
			list->tail->next = item;
			item->prev = list->tail;
		}
		else
		{
			list->head = item;
		}

		list->tail = item;

		++list->count;
	}

	return item;
}

ListItem *
list_prepend(List *list, void *data)
{
	assert(list != NULL);
	assert(list->count != SIZE_MAX);

	ListItem *item = NULL;

	if(list->count < SIZE_MAX)
	{
		item = _list_item_new(list->pool, data);

		item->next = list->head;

		if(list->head)
		{
			list->head->prev = item;
		}
		else
		{
			list->tail = item;
		}

		list->head = item;

		++list->count;
	}

	return item;
}

static ListItem *
_list_prepend_new_sorted(List *list, void *data)
{
	assert(list != NULL);
	assert(list->compare != NULL);
	assert(list->count != SIZE_MAX);

	ListItem *item = _list_item_new(list->pool, data);
	ListItem **ptr = &list->head;
	bool inserted = false;

	while(*ptr && !inserted)
	{
		if(list->compare((*ptr)->data, data) >= 0)
		{
			inserted = true;

			item->next = *ptr;
			item->prev = (*ptr)->prev;
			(*ptr)->prev = item;

			*ptr = item;
		}
		else
		{
			ptr = &(*ptr)->prev->next;
		}
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

	ListItem *item = NULL;

	if(list->count < SIZE_MAX)
	{
		if(list->count)
		{
			if(list->compare(list->tail->data, data) <= 0)
			{
				item = list_append(list, data);
			}
			else
			{
				item = _list_prepend_new_sorted(list, data);
			}
		}
		else
		{
			item = _list_item_new(list->pool, data);

			list->head = item;
			list->tail = item;
			list->count = 1;
		}
	}

	return item;
}

static void
_list_detach(List *list, ListItem *item)
{
	assert(list != NULL);
	assert(item != NULL);

	ListItem **offset = &list->head;
	ListItem **next = &list->tail;

	if(item->prev)
	{
		offset = &item->prev->next;
	}

	if(item->next)
	{
		next = &item->next->prev;
	}

	*offset = item->next;
	*next = item->prev;
}
void
list_remove(List *list, ListItem *item)
{
	assert(list != NULL);
	assert(list->head != NULL);
	assert(item != NULL);

	_list_detach(list, item);
	_list_item_free(list, item);

	--list->count;
}

void
list_remove_by_data(List *list, void *data, bool remove_all)
{
	assert(list != NULL);
	assert(list->compare != NULL);

	ListItem *iter = list->head;
	bool completed = false;

	while(iter && !completed)
	{
		ListItem *next = iter->next;

		if(!list->compare(iter->data, data))
		{
			completed = !remove_all;

			list_remove(list, iter);
		}

		iter = next;
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

	ListItem *begin = offset ? offset : list->head;
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

	list_free(list);

	list->count = 0;
	list->head = NULL;
	list->tail = NULL;
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

