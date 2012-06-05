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

void
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
}

void
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
}

void
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
			list->count--;
			p = iter;

			/* check if there's a previous list item */
			if(iter->prev)
			{
				/* update previous list item */
				iter->prev->next = iter->next;

				if(iter->next)
				{
					iter->next->prev = iter->prev;
				}
			}
			else
			{
				/* update head of list */
				if((list->head = iter->next))
				{
					list->head->prev = NULL;
				}
			}

			/* update tail of list */
			if(iter == list->tail)
			{
				list->tail = iter->prev;
			}

			/* free allocated memory */
			if(list->free)
			{
				list->free(p->data);
			}

			free(p);

			/* get next list item */
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

	iter = list->head;

	while(iter)
	{
		if(list->equals(iter->data, data))
		{
			return true;
		}

		iter = iter->next;
	}

	return false;
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
list_next(ListItem *item)
{
	assert(item != NULL);

	return item->next;
}

inline ListItem *
list_prev(ListItem *item)
{
	assert(item != NULL);

	return item->prev;
}

