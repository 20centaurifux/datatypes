/***************************************************************************
    begin........: May 2012
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
 * \file assocarray.h
 * \brief A generic associative array.
 * \author Sebastian Fedrau <sebastian.fedrau@gmail.com>
 */
#ifndef ASSOCARRAY_H
#define ASSOCARRAY_H

#include <stdbool.h>

#include "datatypes.h"
#include "allocator.h"

/**
 *\struct AssocArray
 *\brief A datatype to create associations between keys and values.
 */
typedef struct
{
	/*! A function to compare two keys. */
	CompareFunc compare_keys;
	/*! Sorted array containing keys. */
	void **keys;
	/*! Array containing values. */
	void **values;
	/*! A function to free keys. */
	FreeFunc free_key;
	/*! A function to free values. */
	FreeFunc free_value;
	/*! Size of the array. */
	size_t size;
	/*! Number of inserted values. */
	size_t count;
} AssocArray;

/**
 *\struct AssocArrayIter
 *\brief A structure to iterate over the elements of an AssocArray.
 */
typedef struct
{
	/*! Pointer to the associated AssocArray. */
	const AssocArray *array;
	/*! Index of the current item. */
	ssize_t offset;
} AssocArrayIter;

/**
 *\param compare_keys function to compare of two keys
 *\param free_key function to free keys or NULL
 *\param free_value function to free values or NULL
 *\return a new AssocArray
 *
 * Creates a new AssocArray.
 */
AssocArray *assoc_array_new(CompareFunc compare_keys, FreeFunc free_key, FreeFunc free_value);

/**
 *\param array an AssocArray
 *\param compare_keys function to compare of two keys
 *\param free_key function to free keys or NULL
 *\param free_value function to free values or NULL
 *
 * Initializes an AssocArray.
 */
void assoc_array_init(AssocArray *array, CompareFunc compare_keys, FreeFunc free_key, FreeFunc free_value);

/**
 *\param array an AssocArray
 *
 * Frees all keys, values and the array pointer.
 */
void assoc_array_destroy(AssocArray *array);

/**
 *\param array an AssocArray
 *
 * Frees all keys and values without freeing the array pointer.
 */
void assoc_array_free(AssocArray *array);

/**
 *\param array an AssocArray
 *
 * Removes all elements from the AssocArray.
 */
void assoc_array_clear(AssocArray *array);

/**
 *\param array an AssocArray
 *\param key a key to insert
 *\param value the value to associate with the key
 *\param overwrite_key true to overwrite already exisiting keys
 *
 * Inserts a new key and value in the AssocArray. If overwrite_key is set an existing key is
 * freed using the specified free_key function before it gets replaced.
 */
void assoc_array_set(AssocArray *array, void * restrict key, void * restrict value, bool overwrite_key);

/**
 *\param array an AssocArray
 *\param key key of the element to remove
 *
 * Removes an element from the AssocArray.
 */
void assoc_array_remove(AssocArray *array, const void *key);

/**
 *\param array an AssocArray
 *\param key a key
 *\return the value associated to the key or NULL
 *
 * Looks up a key in the AssocArray and returns its value.
 */
void *assoc_array_lookup(const AssocArray *array, const void *key);

/**
 *\param array an AssocArray
 *\param key a key
 *\return true if given key does exist
 *
 * Checks if a key does exist.
 */
bool assoc_array_key_exists(const AssocArray *array, const void *key);

/**
 *\param array an AssocArray
 *\return number of stored elements
 *
 * Gets the number of stored elements.
 */
size_t assoc_array_count(const AssocArray *array);

/**
 *\param array an AssocArray
 *\return size of the array
 *
 * Gets the size of the array.
 */
size_t assoc_array_size(const AssocArray *array);

/**
 *\param array an AssocArray
 *\param iter an uninitialized AssocArrayIter
 *
 * Initializes a key/value pair iterator and associates it with the array. Modifying the array while
 * using the iterator might lead to undefined behaviour.
 */
void assoc_array_iter_init(const AssocArray *array, AssocArrayIter *iter);

/**
 *\param iter an AssocArrayIter
 *\return false if end of the AssocArray has been reached
 *
 * Goes to next element of an AssocArray.
 */
bool assoc_array_iter_next(AssocArrayIter *iter);

/**
 *\param iter an AssocArrayIter
 *\return key of current element
 *
 * Retrieves the key of the current element.
 */
void *assoc_array_iter_get_key(const AssocArrayIter *iter);

/**
 *\param iter an AssocArrayIter
 *\return value of current element
 *
 * Retrieves the value of the current element.
 */
void *assoc_array_iter_get_value(const AssocArrayIter *iter);

#endif

