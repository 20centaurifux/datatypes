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
 * \file slist.h
 * \brief A singly-linked list.
 * \author Sebastian Fedrau <lord-kefir@arcor.de>
 * \version 0.1.0
 * \date 26. June 2012
 */

#include "datatypes.h"
#include "allocator.h"

#ifndef __SLIST_H__
#define __SLIST_H__

#include <stdint.h>
#include <stdbool.h>

/**
 * \struct SListItem
 * \brief Structure holding list item data.
 */
typedef struct _SListItem
{
	/*! Data of the list item. */
	void *data;
	/*! Pointer to next list element or NULL. */
	struct _SListItem *next;
} SListItem;

/**
 * \struct SList
 * \brief A singly-linked list.
 */
typedef struct _SList
{
	/*! Head of the list. */
	SListItem *head;
	/*! Tail of the list. */
	SListItem *tail;
	/*! Number of stored items. */
	size_t count;
	/*! A function to compare data of two list items. */
	EqualFunc equals;
	/*! A function to free item data. */
	FreeFunc free;
	/*! A memory allocator. */
	Allocator *allocator;
} SList;

/**
 *\param equals function to compare item data
 *\param free function to free item data or NULL
 *\param allocator a user-defined memory allocator for creating/destroying SListItems or NULL
 *\return a new SList
 *
 * Creates a new SList.
 */
SList *slist_new(EqualFunc equals, FreeFunc free, Allocator *allocator);

/**
 *\param list a SList
 *\param equals function to compare item data
 *\param free function to free item data OR NULL
 *\param allocator a user-defined memory allocator for creating/destroying SListItems or NULL
 *
 * Initializes a new SList.
 */void slist_init(SList *list, EqualFunc equals, FreeFunc free, Allocator *allocator);

/**
 *\param list a SList
 *
 * Destroys all items in the list. Frees also memory allocated for the SList instance.
 */
void slist_destroy(SList *list);

/**
 *\param list a SList
 *
 * Destroys all items in the list.
 */
void slist_free(SList *list);

/**
 *\param list a SList
 *\param data data to append
 *\return a new SListItem
 *
 * Appends data to the list.
 */
SListItem *slist_append(SList *list, void *data);

/**
 *\param list a SList
 *\param data data to prepend
 *\return a new SListItem
 *
 * Prepends data to the list.
 */
SListItem *slist_prepend(SList *list, void *data);

/**
 *\param list a List
 *\param data data to insert
 *\param compare function to compare list item data
 *\return a new ListItem
 *
 * Inserts data into list using the given comparison function to determine its position.
 */
SListItem *slist_insert_sorted(SList *list, void *data, CompareFunc compare);

/**
 *\param list a SList
 *\return head of the list
 *
 * Gets the head of the list.
 */
SListItem *slist_head(SList *list);

/**
 *\param list a SList
 *\return number of items
 *
 * Gets the number of items.
 */
size_t slist_count(SList *list);

/**
 *\param list a SList
 *\return true if list is empty
 *
 * Checks if a list is empty.
 */
bool slist_empty(SList *list);

/**
 *\param list a SList
 *\param item a SListItem
 *
 * Removes an item from the list.
 */
void slist_remove(SList *list, SListItem *item);

/**
 *\param list a SList
 *\param data data
 *\param remove_all true to remove all items with associated data
 *
 * Removes first item (or all items) with associated data from the list.
 */
void slist_remove_by_data(SList *list, void *data, bool remove_all);

/**
 *\param list a SList
 *\return list item data
 *
 * Removes first element from list and returns data.
 */
void *slist_pop(SList *list);

/**
 *\param list a SList
 *\param data data to search
 *\return true if data exists in list
 *
 * Test if list contains given data.
 */
bool slist_contains(SList *list, void *data);

/**
 *\param list a SList
 *
 * Clears a list.
 */
void slist_clear(SList *list);

/**
 *\param list a SList
 *\param offset position to start search from
 *\param data data to search
 *\return found SListItem or NULL
 *
 * Searches for given data.
 */
SListItem *slist_find(SList *list, SListItem *offset, void const *data);

/*! Get next list item. */
#define slist_item_next(item) item->next

/*! Gets data of the specified SListItem. */
#define slist_item_get_data(item) item->data

/*! Sets data of the specified SListItem. */
#define slist_item_set_data(item, value) item->data = value

/**
 *\param list a SList
 *\param item a SListItem
 *\param compare function to compare list item data
 *
 * Arrange given SListItem in order using the given comparison function to determine its position.
 */
void slist_reorder(SList *list, SListItem *item, CompareFunc compare);
#endif

