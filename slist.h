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
 * \file slist.h
 * \brief A singly-linked list.
 * \author Sebastian Fedrau <sebastian.fedrau@gmail.com>
 */
#include "datatypes.h"

#ifndef SLIST_H
#define SLIST_H

#include <stdint.h>
#include <stdbool.h>

#include "pool.h"

/**
 * \struct SListItem
 * \brief Structure holding a list item.
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
	CompareFunc compare;
	/*! A function to free item data. */
	FreeFunc free;
	/*! A memory pool. */
	Pool *pool;
} SList;

/**
 *\param compare function to compare item data
 *\param free function to free item data or NULL
 *\param pool a user-defined memory pool for creating/destroying SListItems or NULL
 *\return a new SList
 *
 * Creates a new SList.
 */
SList *slist_new(CompareFunc compare, FreeFunc free, Pool *pool);

/**
 *\param list a SList
 *\param compare function to compare item data
 *\param free function to free item data OR NULL
 *\param pool a user-defined memory pool for creating/destroying SListItems or NULL
 *
 * Initializes a new SList.
 */
void slist_init(SList *list, CompareFunc compare, FreeFunc free, Pool *pool);

/**
 *\param list a SList
 *
 * Frees all items in the list and the list pointer.
 */
void slist_destroy(SList *list);

/**
 *\param list a SList
 *
 * Frees all items in the list without freeing the list pointer.
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
 *\return a new ListItem
 *
 * Inserts data into the list using the associated compare function to determine its position.
 */
SListItem *slist_insert_sorted(SList *list, void *data);

/**
 *\param list a SList
 *\return head of the list
 *
 * Gets the head of the list.
 */
SListItem *slist_head(const SList *list);

/**
 *\param list a SList
 *\return number of items
 *
 * Gets the number of items.
 */
size_t slist_count(const SList *list);

/**
 *\param list a SList
 *\return true if list is empty
 *
 * Checks if a list is empty.
 */
bool slist_empty(const SList *list);

/**
 *\param list a SList
 *\param item a SListItem
 *
 * Removes a list item from the list.
 */
void slist_remove(SList *list, SListItem *item);

/**
 *\param list a SList
 *\param data data
 *\param remove_all true to remove all items with associated data
 *
 * Removes first list item (or all items) with associated data from the list.
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
 * Tests if a list contains the specified data.
 */
bool slist_contains(const SList *list, void *data);

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
 * Searches for an element in the list.
 */
SListItem *slist_find(const SList *list, SListItem *offset, void const *data);

/*! Get next list item. */
#define slist_item_next(item) item->next

/*! Gets data of the specified SListItem. */
#define slist_item_get_data(item) item->data

/*! Sets data of the specified SListItem. */
#define slist_item_set_data(item, value) item->data = value

/**
 *\param list a SList
 *\param item a SListItem
 *
 * Sets the associated list item to NULL and frees memory.
 */
void slist_item_free_data(const SList *list, SListItem *item);

/**
 *\param list a SList
 *\param item a SListItem
 *
 * Arrange given SListItem in order using the associated compare function to determine its position.
 */
void slist_reorder(SList *list, SListItem *item);
#endif

