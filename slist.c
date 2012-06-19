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

static bool
_slist_find(SList *list, void const *data, SListIter *iter)
{
	SListItem *p;

	memset(iter, 0, sizeof(SListIter));

	p = list->head;

	while(p)
	{
		iter->next = p->next;

		if(list->equals(p->data, data))
		{
			iter->cur = p;

			return true;
		}

		iter->prev = p;
		p = p->next;
	}

	iter->finished = true;

	return false;
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

	list->count--;
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

	list->count++;

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

	list->count++;

	return item;
}

void
slist_remove(SList *list, SListIter *iter)
{
	SListItem *p;

	assert(list != NULL);
	assert(list->head != NULL);
	assert(item != NULL);

	p = iter->cur;

	/* remove list item */
	_slist_remove(list, iter->prev, iter->cur);

	/* free allocated memory */
	if(list->free)
	{
		list->free(p->data);
	}

	if(list->allocator)
	{
		list->allocator->free(list->allocator, p);
	}
	else
	{
		free(p);
	}

	list->count--;
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

		list->count--;
	}

	return data;
}

bool
slist_contains(SList *list, void *data)
{
	SListIter iter;

	assert(list != NULL);
	assert(list->equals != NULL);

	return _slist_find(list, data, &iter);
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

bool
slist_find(SList *list, void const *data, SListIter *iter)
{
	return _slist_find(list, data, iter);
}

inline void
slist_iter_init(SList *list, SListIter *iter)
{
	assert(list != NULL);

	if(list->head)
	{
		iter->prev = NULL;
		iter->next = list->head->next;
		iter->cur = list->head;
		iter->finished = false;
		iter->head = false;
	}
	else
	{
		memset(iter, 0, sizeof(SListIter));
		iter->finished = true;
	}
}

inline uint32_t
slist_count(SList *list)
{
	assert(list != NULL);

	return list->count;
}

inline bool
slist_empty(SList *list)
{
	assert(list != NULL);

	return list->head ? true : false;
}

inline bool
slist_iter_next(SListIter *iter)
{
	SListItem *next;

	assert(item != NULL);

	if(iter->finished)
	{
		return false;
	}

	if(iter->head)
	{
		if((next = iter->next))
		{
			iter->prev = iter->cur;
			iter->cur = next;
			iter->next = next->next;
		}
		else
		{
			iter->finished = true;

			return false;
		}
	}
	else
	{
		iter->head = true;
	}

	return true;
}

inline void *
slist_iter_get_data(SListIter *iter)
{
	assert(item != NULL);

	if(iter->cur)
	{
		return iter->cur->data;
	}

	return NULL;
}

inline void
slist_iter_set_data(SListIter *iter, void *data)
{
	assert(item != NULL);

	if(!iter->finished)
	{
		iter->cur->data = data;
	}
}

