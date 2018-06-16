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
		fprintf(stderr, "Couldn't allocate memory.\n");
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

void
slist_free(SList *list)
{
	assert(list != NULL);

	SListItem *iter = list->head;

	while(iter)
	{
		SListItem *item = iter;

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
			fprintf(stderr, "Couldn't allocate memory.\n");
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

	SListItem *item = _slist_item_new(list->pool, data);

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
	assert(list != NULL);
	assert(list->count != SIZE_MAX);

	SListItem *item = _slist_item_new(list->pool, data);

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
slist_insert_sorted(SList *list, void *data)
{
	assert(list != NULL);
	assert(list->compare != NULL);
	assert(list->count != SIZE_MAX);

	SListItem *iter = list->head;
	SListItem *item;

	if(iter)
	{
		if(list->compare(list->tail->data, data) <= 0)
		{
			item = slist_append(list, data);
		}
		else if(list->count == 1)
		{
			item = slist_prepend(list, data);
		}
		else
		{
			item = _slist_item_new(list->pool, data);

			SListItem *prev = NULL;

			while(iter)
			{
				if(list->compare(iter->data, data) >= 0)
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
	}
	else
	{
		item = _slist_item_new(list->pool, data);
		list->head = item;
		list->tail = item;
	}

	++list->count;

	return item;
}

static void
_slist_remove(SList *list, SListItem *prev, SListItem *item)
{
	assert(list != NULL);
	assert(item != NULL);

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

	--list->count;
}

void
slist_remove(SList *list, SListItem *item)
{
	assert(list != NULL);
	assert(list->head != NULL);
	assert(item != NULL);

	SListItem *iter = list->head;
	SListItem *prev = NULL;

	while(iter)
	{
		if(iter == item)
		{
			_slist_remove(list, prev, iter);
			break;
		}

		prev = iter;
		iter = iter->next;
	}
}

void
slist_remove_by_data(SList *list, void *data, bool remove_all)
{
	assert(list != NULL);
	assert(list->compare != NULL);

	SListItem *iter = list->head;
	SListItem *next;
	SListItem *prev = NULL;

	while(iter)
	{
		if(!list->compare(iter->data, data))
		{
			next = iter->next;
			_slist_remove(list, prev, iter);
			iter = next;

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
	assert(list != NULL);

	SListItem *item = list->head;
	void *data = NULL;

	if(item)
	{
		list->head = list->head->next;

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

	SListItem *iter = offset ? offset : list->head;

	while(iter)
	{
		if(!list->compare(iter->data, data))
		{
			return iter;
		}

		iter = iter->next;
	}

	return NULL;
}

bool
slist_contains(const SList *list, void *data)
{
	assert(list != NULL);
	assert(list->compare != NULL);

	return _slist_find(list, NULL, data) != NULL;
}

void
slist_clear(SList *list)
{
	assert(list != NULL);

	SListItem *iter = list->head;

	while(iter)
	{
		SListItem *next = iter->next;

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

void
slist_reorder(SList *list, SListItem *item)
{
	assert(list != NULL);
	assert(list->compare != NULL);
	assert(item != NULL);

	SListItem *iter = list->head;
	SListItem *prev = NULL;

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

	iter = list->head;
	prev = NULL;

	bool inserted = false;

	while(iter)
	{
		if(list->compare(iter->data, item->data) >= 0)
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
		if(list->tail)
		{
			list->tail->next = item;
		}

		list->tail = item;
		item->next = NULL;
	}
}

