/***************************************************************************
    begin........: June 2012
    copyright....: Sebastian Fedrau
    email........: lord-kefir@arcor.de
 ***************************************************************************/

/***************************************************************************
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 ***************************************************************************/
/*!
 * \file list.h
 * \brief A doubly-linked list.
 * \author Sebastian Fedrau <lord-kefir@arcor.de>
 * \version 0.1.0
 * \date 26. June 2012
 */

#ifndef __LIST_H__
#define __LIST_H__

#include <stdint.h>
#include <stdbool.h>

#include "datatypes.h"
#include "allocator.h"

/**
 * \struct ListItem
 * \brief Structure holding list item data.
 */
typedef struct _ListItem
{
	/*! Store data. */
	void *data;
	/*! Pointer to next list item. */
	struct _ListItem *next;
	/*! Pointer to previous list item. */
	struct _ListItem *prev;
} ListItem;

/**
 * \struct List
 * \brief A doubly-linked list.
 */
typedef struct
{
	/*! Head of the list. */
	ListItem *head;
	/*! Tail of the list. */
	ListItem *tail;
	/*! Number of stored items. */
	uint32_t count;
	/*! A function to compare data of two list items. */
	EqualFunc equals;
	/*! A function to free item data. */
	FreeFunc free;
	/*! A memory allocator. */
	Allocator *allocator;
} List;

/**
 *\param equals function to compare item data
 *\param free function to free item data or NULL
 *\param allocator a user-defined memory allocator for creating/destroying ListItems  or NULL
 *\return a new List
 *
 * Creates a new List.
 */
List *list_new(EqualFunc equals, FreeFunc free, Allocator *allocator);

/**
 *\param list a List
 *\param equals function to compare item data
 *\param free function to free item data or NULL
 *\param allocator a user-defined memory allocator for creating/destroying ListItems  or NULL
 *
 * Initializes a List.
 */
void list_init(List *list, EqualFunc equals, FreeFunc free, Allocator *allocator);

/**
 *\param list a List
 *
 * Destroys all items in the list. Frees also memory allocated for the List instance.
 */
void list_destroy(List *list);

/**
 *\param list a List
 *
 * Destroys all items in the list.
 */
void list_free(List *list);

/**
 *\param list a List
 *\param data data to append
 *\return a new ListItem
 *
 * Appends data to the list.
 */
ListItem *list_append(List *list, void *data);

/**
 *\param list a List
 *\param data data to prepend
 *\return a new ListItem
 *
 * Prepends data to the list.
 */
ListItem *list_prepend(List *list, void *data);

/**
 *\param list a List
 *\param data data to insert
 *\param compare function to compare list item data
 *\return a new ListItem
 *
 * Inserts data into list using the given comparison function to determine its position.
 */
ListItem *list_insert_sorted(List *list, void *data, CompareFunc compare);

/**
 *\param list a List
 *\return head of the list
 *
 * Gets the head of the list.
 */
ListItem *list_head(List *list);

/**
 *\param list a List
 *\return tail of the list
 *
 * Gets the tail of the list.
 */
ListItem *list_tail(List *list);

/**
 *\param list a List
 *\return number of items
 *
 * Gets the number of items.
 */
uint32_t list_count(List *list);

/**
 *\param list a List
 *\return true if list is empty
 *
 * Checks if a list is empty.
 */
bool list_empty(List *list);

/**
 *\param list a List
 *\param item a ListItem
 *
 * Removes an item from the list.
 */
void list_remove(List *list, ListItem *item);

/**
 *\param list a List
 *\param data data
 *\param remove_all true to remove all items with associated data
 *
 * Removes first item (or all items) with associated data from the list.
 */
void list_remove_by_data(List *list, void *data, bool remove_all);

/**
 *\param list a List
 *\return list item data
 *
 * Removes first element from list and returns data.
 */
void *list_pop(List *list);

/**
 *\param list a List
 *\param data data
 *\return true if data exists in list
 *
 * Tests if list contains given data.
 */
bool list_contains(List *list, void *data);

/**
 *\param list a List
 *
 * Clears a list.
 */
void list_clear(List *list);

/**
 *\param list a List
 *\param offset position to start search from
 *\param data data to search
 *\return found ListItem or NULL
 *
 * Searches for given data.
 */
ListItem *list_find(List *list, ListItem *offset, void const *data);

/*! Gets next list item. */
#define list_item_next(item) item->next

/*! Gets previous list item. */
#define list_item_prev(item) item->prev

/*! Gets data of the specified ListItem. */
#define list_item_get_data(item) item->data

/*! Sets data of the specified ListItem. */
#define list_item_set_data(item, value) item->data = value

/**
 *\param list a List
 *\param item a ListItem
 *\param compare function to compare list item data
 *
 * Arrange given ListItem in order using the given comparison function to determine its position.
 */
void list_reorder(List *list, ListItem *item, CompareFunc compare);

#endif

