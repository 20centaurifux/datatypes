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
#include <sys/types.h>

#include "datatypes.h"

/**
 *\struct AssocArray
 *\brief A datatype to create associations between keys and values.
 */
typedef struct _AssocArray
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
	/*! Found key-value pair. */
	struct _AssocArrayPair
	{
		/*! Reference to the array. */
		const struct _AssocArray *array;
		/*! Index of the found key-value pair. */
		ssize_t offset;
	} pair;
} AssocArray;

/*! A found key-value pair. */
typedef struct _AssocArrayPair AssocArrayPair;

/*! A structure to iterate over the elements of an AssocArray. */
typedef struct _AssocArrayPair AssocArrayIter;

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
 *\return the found key-value pair or NULL.
 *
 * Looks up a key-value pair in the AssocArray.
 */
AssocArrayPair *assoc_array_lookup(AssocArray *array, const void *key);

/**
 *\param pair a key-value pair
 *\return key of the pair
 *
 * Retrieves the key of a key-value pair.
 */
void *assoc_array_pair_get_key(const AssocArrayPair *pair);

/*! Accesses the key of a key-value pair directly. */
#define assoc_array_pair_key(p) p->array->keys[p->offset]

/**
 *\param pair a key-value pair
 *\return value of the pair
 *
 * Retrieves the value of a key-value pair.
 */
void *assoc_array_pair_get_value(const AssocArrayPair *pair);

/*! Accesses the value of a key-value pair directly. */
#define assoc_array_pair_value(p) p->array->values[p->offset]

/**
 *\param pair a AssocArrayPair
 *\param value new value to set
 *
 * Overwrites the value of a key-value pair.
 */
void assoc_array_pair_set_value(AssocArrayPair *pair, void *value);

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

/*! Accesses the key of the current element directly. */
#define assoc_array_iter_key(iter) assoc_array_pair_key(iter)

/**
 *\param iter an AssocArrayIter
 *\return value of current element
 *
 * Retrieves the value of the current element.
 */
void *assoc_array_iter_get_value(const AssocArrayIter *iter);

/*! Accesses the value of the current element directly. */
#define assoc_array_iter_value(iter) assoc_array_pair_value(iter)

#endif

