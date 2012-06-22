#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "list.h"

/*
 *	helpers:
 */
static inline ListItem *
_list_item_new(Allocator *allocator, void *data)
{
	ListItem *item = NULL;

	if(allocator)
	{
		item = (ListItem *)allocator->alloc(allocator);
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
_list_find(List *list, ListItem *offset, void const *data)
{
	ListItem *begin;
	ListItem *end;

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
		if(list->equals(begin->data, data))
		{
			return begin;
		}

		if(list->equals(end->data, data))
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
list_new(EqualFunc equals, FreeFunc free, Allocator *allocator)
{
	List *list;

	if(!(list = (List *)malloc(sizeof(List))))
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		abort();
	}

	list_init(list, equals, free, allocator);

	return list;
}

void
list_init(List *list, EqualFunc equals, FreeFunc free, Allocator *allocator)
{
	assert(equals != NULL);

	memset(list, 0, sizeof(List));
	list->equals = equals;
	list->free = free;
	list->allocator = allocator;
}

void
list_free(List *list)
{
	ListItem *iter;
	ListItem *item;

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
list_destroy(List *list)
{
	list_free(list);
	free(list);
}

ListItem *
list_append(List *list, void *data)
{
	ListItem *item;

	assert(list != NULL);

	item = _list_item_new(list->allocator, data);

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

	list->count++;

	return item;
}

ListItem *
list_prepend(List *list, void *data)
{
	ListItem *item;

	assert(list != NULL);

	item = _list_item_new(list->allocator, data);

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

	list->count++;

	return item;
}

ListItem *
list_insert_sorted(List *list, void *data, CompareFunc compare)
{
	ListItem *iter;
	ListItem *item;

	assert(list != NULL);
	assert(compare != NULL);

	if((iter = list->head))
	{
		/* test if new item can be appended */
		if(compare(list->tail->data, data) <= 0)
		{
			return list_append(list, data);
		}
		else if(list->count == 1)
		{
			/* prepend item */
			return list_prepend(list, data);
		}

		/* insert list item into list */
		item = _list_item_new(list->allocator, data);

		while(iter)
		{
			if(compare(iter->data, data) >= 0)
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
		item = _list_item_new(list->allocator, data);
		list->head = item;
		list->tail = item;
	}

	list->count++;

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
list_remove_by_data(List *list, void *data, bool remove_all)
{
	ListItem *iter;
	ListItem *next;

	assert(list != NULL);
	assert(list->equals != NULL);

	iter = list->head;

	while(iter)
	{
		if(list->equals(iter->data, data))
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
list_contains(List *list, void *data)
{
	assert(list != NULL);
	assert(list->equals != NULL);

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

ListItem *
list_find(List *list, ListItem *offset, void const *data)
{
	return _list_find(list, offset, data);
}

inline ListItem *
list_head(List *list)
{
	assert(list != NULL);

	return list->head;
}

inline ListItem *
list_tail(List *list)
{
	assert(list != NULL);

	return list->tail;
}

inline uint32_t
list_count(List *list)
{
	assert(list != NULL);

	return list->count;
}

inline bool
list_empty(List *list)
{
	assert(list != NULL);

	return list->head ? true : false;
}

void
list_reorder(List *list, ListItem *item, CompareFunc compare)
{
	ListItem *iter;

	assert(list != NULL);
	assert(item != NULL);

	if(list->head == item)
	{
		return;
	}

	/* detach list item */
	_list_detach(list, item);

	/* insert list item */
	iter = list->head;

	while(iter)
	{
		if(compare(iter->data, item->data) >= 0)
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

