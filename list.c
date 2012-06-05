#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "list.h"

/*
 *	helpers:
 */
static inline ListItem *
_list_item_new(void *data)
{
	ListItem *item;

	if(!(item = (ListItem *)malloc(sizeof(ListItem))))
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
_list_find(List *list, ListItem *offset, void *data)
{
	ListItem *iter;

	if(offset)
	{
		iter = offset->next;
	}
	else
	{
		iter = list->head;
	}

	while(iter)
	{
		if(list->equals(iter->data, data))
		{
			return iter;
		}

		iter = iter->next;
	}

	return NULL;
}

/*
 *	public:
 */
List *
list_new(EqualFunc equals, FreeFunc free)
{
	List *list;

	assert(equals != NULL);

	if(!(list = (List *)malloc(sizeof(List))))
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		abort();
	}

	memset(list, 0, sizeof(List));
	list->equals = equals;
	list->free = free;

	return list;
}

void
list_destroy(List *list)
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
			list->free(item);
		}

		free(item);
	}
}

ListItem *
list_append(List *list, void *data)
{
	ListItem *item;

	assert(list != NULL);

	item = _list_item_new(data);

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

	item = _list_item_new(data);

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
			list_append(list, data);
			return;
		}
		else if(list->count == 1)
		{
			/* prepend item */
			list_prepend(list, data);
			return;
		}

		/* insert list item into list */
		item = _list_item_new(data);

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
		item = _list_item_new(data);
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

	free(p);

	list->count--;
}

void
list_remove_by_data(List *list, void *data, bool remove_all)
{
	ListItem *iter;
	ListItem *p = NULL;

	assert(list != NULL);
	assert(list->equals != NULL);

	iter = list->head;

	while(iter)
	{
		if(list->equals(iter->data, data))
		{
			p = iter;
			list_remove(list, iter);
			iter = iter->next;

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
		free(item);
		list->count--;
	}

	return data;
}

bool
list_contains(List *list, void *data)
{
	ListItem *iter;

	assert(list != NULL);
	assert(list->equals != NULL);

	return _list_find(list, NULL, data) ? true : false;
}

ListItem *
list_find(List *list, ListItem *offset, void *data)
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

inline ListItem *
list_item_next(ListItem *item)
{
	assert(item != NULL);

	return item->next;
}

inline ListItem *
list_item_prev(ListItem *item)
{
	assert(item != NULL);

	return item->prev;
}

inline void *
list_item_get_data(ListItem *item)
{
	assert(item != NULL);

	return item->data;
}

inline void
list_item_set_data(ListItem *item, void *data)
{
	assert(item ! NULL);

	item->data = data;
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

