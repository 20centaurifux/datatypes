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
 * \file slist.c
 * \brief Singly-linked list.
 * \author Sebastian Fedrau <sebastian.fedrau@gmail.com>
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "slist.h"

SList *
slist_new(CompareFunc compare, FreeFunc free, Pool *pool)
{
	assert(compare != NULL);

	SList *list = (SList *)malloc(sizeof(SList));

	if(!list)
	{
		perror("malloc()");
		abort();
	}

	slist_init(list, compare, free, pool);

	return list;
}

void
slist_init(SList *list, CompareFunc compare, FreeFunc free, Pool *pool)
{
	assert(list != NULL);
	assert(compare != NULL);

	memset(list, 0, sizeof(SList));
	list->compare = compare;
	list->free = free;
	list->pool = pool;
}

static void
_slist_item_free(SList *list, SListItem *item)
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
slist_free(SList *list)
{
	assert(list != NULL);

	SListItem *iter = list->head;

	while(iter)
	{
		SListItem *item = iter;

		iter = iter->next;

		_slist_item_free(list, item);
	}
}

void
slist_destroy(SList *list)
{
	assert(list != NULL);

	slist_free(list);
	free(list);
}

static SListItem *
_slist_item_new(Pool *pool, void *data)
{
	SListItem *item = NULL;

	if(pool)
	{
		item = (SListItem *)pool->alloc(pool);
	}
	else
	{
		item = (SListItem *)malloc(sizeof(SListItem));

		if(!item)
		{
			perror("malloc()");
			abort();
		}
	}

	memset(item, 0, sizeof(SListItem));
	item->data = data;

	return item;
}

SListItem *
slist_append(SList *list, void *data)
{
	assert(list != NULL);
	assert(list->count != SIZE_MAX);

	SListItem *item = NULL;
	
	if(list->count < SIZE_MAX)
	{
		item = _slist_item_new(list->pool, data);

		if(list->head)
		{
			list->tail->next = item;
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

SListItem *
slist_prepend(SList *list, void *data)
{
	assert(list != NULL);
	assert(list->count != SIZE_MAX);

	SListItem *item = NULL;

	if(list->count < SIZE_MAX)
	{
		item = _slist_item_new(list->pool, data);

		if(!list->tail)
		{
			list->tail = item;
		}

		item->next = list->head;
		list->head = item;

		++list->count;
	}

	return item;
}

static SListItem *
_slist_prepend_new_sorted(SList *list, void *data)
{
	assert(list != NULL);
	assert(list->compare != NULL);
	assert(list->count != SIZE_MAX);

	SListItem *item = _slist_item_new(list->pool, data);
	SListItem **ptr = &list->head;
	bool inserted = false;

	while(*ptr && !inserted)
	{
		if(list->compare((*ptr)->data, data) >= 0)
		{
			inserted = true;

			item->next = *ptr;
			*ptr = item;
		}
		else
		{
			ptr = &(*ptr)->next;
		}
	}

	++list->count;

	return item;
}

SListItem *
slist_insert_sorted(SList *list, void *data)
{
	assert(list != NULL);
	assert(list->compare != NULL);
	assert(list->count != SIZE_MAX);

	SListItem *item = NULL;

	if(list->count < SIZE_MAX)
	{
		if(list->count)
		{
			if(list->compare(list->tail->data, data) <= 0)
			{
				item = slist_append(list, data);
			}
			else
			{
				item = _slist_prepend_new_sorted(list, data);
			}
		}
		else
		{
			item = _slist_item_new(list->pool, data);

			list->head = item;
			list->tail = item;
			list->count = 1;
		}
	}

	return item;
}

void
slist_remove(SList *list, SListItem *item)
{
	assert(list != NULL);
	assert(list->head != NULL);
	assert(item != NULL);

	SListItem **ptr = &list->head;
	SListItem *tail = NULL;
	bool found = false;

	while(*ptr && !found)
	{
		if(*ptr == item)
		{
			found = true;

			if(!item->next)
			{
				list->tail = tail;
			}

			*ptr = item->next;
			_slist_item_free(list, item);

			--list->count;
		}
		else
		{
			tail = *ptr;
			ptr = &(*ptr)->next;
		}
	}
}

void
slist_remove_by_data(SList *list, void *data, bool remove_all)
{
	assert(list != NULL);
	assert(list->compare != NULL);

	SListItem **ptr = &list->head;
	SListItem *tail = NULL;
	bool completed = false;

	while(*ptr && !completed)
	{
		if(!list->compare((*ptr)->data, data))
		{
			completed = !remove_all;

			SListItem *next = (*ptr)->next;

			if(!next)
			{
				list->tail = tail;
			}

			_slist_item_free(list, *ptr);
			*ptr = next;

			--list->count;
		}
		else
		{
			tail = *ptr;
			ptr = &(*ptr)->next;
		}
	}
}

void *
slist_pop(SList *list)
{
	assert(list != NULL);

	void *data = NULL;
	SListItem *item = list->head;

	if(item)
	{
		list->head = item->next;

		if(!list->head)
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

static SListItem *
_slist_find(const SList *list, SListItem *offset, void const *data)
{
	assert(list != NULL);
	assert(list->compare != NULL);

	SListItem *iter = offset ? offset : list->head;
	SListItem *found = NULL;

	while(iter && !found)
	{
		if(!list->compare(iter->data, data))
		{
			found = iter;
		}

		iter = iter->next;
	}

	return found;
}

bool
slist_contains(const SList *list, const void *data)
{
	assert(list != NULL);
	assert(list->compare != NULL);

	return _slist_find(list, NULL, data) != NULL;
}

void
slist_clear(SList *list)
{
	assert(list != NULL);

	slist_free(list);

	list->count = 0;
	list->head = NULL;
	list->tail = NULL;
}

SListItem *
slist_find(const SList *list, SListItem *offset, void const *data)
{
	assert(list != NULL);

	return _slist_find(list, offset, data);
}

size_t
slist_count(const SList *list)
{
	assert(list != NULL);

	return list->count;
}

SListItem *
slist_head(const SList *list)
{
	assert(list != NULL);

	return list->head;
}

bool
slist_empty(const SList *list)
{
	assert(list != NULL);

	return list->head == NULL;
}

void
slist_item_free_data(const SList *list, SListItem *item)
{
	assert(list != NULL);
	assert(item != NULL);

	if(list->free && item->data)
	{
		list->free(item->data);
	}

	item->data = NULL;
}

