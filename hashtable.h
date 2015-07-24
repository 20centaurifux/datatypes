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
/*!
 * \file hashtable.h
 * \brief A generic hashtable.
 * \author Sebastian Fedrau <sebastian.fedrau@gmail.com>
 * \version 0.1.0
 * \date 26. June 2012
 */

#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

#include <stdbool.h>

#include "datatypes.h"
#include "allocator.h"

/*! Specifies the type of the hash function which is passed to hashtable_new() or hashtable_init(). */
typedef uint32_t (*HashFunc)(const char *plain);

/**
 *\struct HashTable
 *\brief A datatype to create associations between keys and values.
 */
typedef struct
{
	/*! A function to check equality of two keys. */
	EqualFunc compare_keys;
	/*! A function to free keys. */
	FreeFunc free_key;
	/*! A function to free values. */
	FreeFunc free_value;
	/*! A function to create a hash from a value. */
	HashFunc hash;
	/*! Size of the hash table. */
	size_t size;
	/**
	 *\struct _Bucket
	 *\brief A singly-linked list implementation storing keys & values.
	 *
	 *\var buckets
	 *\brief An array containing pointers to lists.
	 */
	struct _Bucket
	{
		/*! Key of the element. */
		void *key;
		/*! Value of the element. */
		void *data;
		/*! Pointer to next list element or NULL. */
		struct _Bucket *next;
	} **buckets;
	/*! Number of stored elements. */
	size_t count;
	/*! Allocator used to create/destroy list elements. */
	Allocator *allocator;
} HashTable;

/**
 *\struct HashTableIter
 *\brief A structure to iterate over the elements of a HashTable.
 */
typedef struct
{
	/*! Pointer to the associated HashTable. */
	HashTable *table;
	/*! Index of current bucket. */
	size_t offset;
	/*! Pointer to current list element. */
	struct _Bucket *liter;
	/*! true if iteration has been completed. */
	bool finished;
} HashTableIter;

/**
 *\param plain plain text
 *\return a hash value
 *
 * Hash of the given plain text.
 */
uint32_t str_hash(const char *plain);

/**
 *\param size size of the hash table
 *\param hash_func function to create hash from a key
 *\param compare_keys function to check equality of two keys
 *\param free_key function to free keys or NULL
 *\param free_value function to free values or NULL
 *\return a new HashTable
 *
 * Creates a new HashTable.
 */
HashTable *hashtable_new(size_t size, HashFunc hash_func, EqualFunc compare_keys, FreeFunc free_key, FreeFunc free_value);

/**
 *\param table a HashTable
 *\param size size of the hash table
 *\param hash_func function to create hash from a key
 *\param compare_keys function to check equality of two keys
 *\param free_key function to free keys or NULL
 *\param free_value function to free values or NULL
 *
 * Initializes a HashTable.
 */
void hashtable_init(HashTable *table, size_t size, HashFunc hash_func, EqualFunc compare_keys, FreeFunc free_key, FreeFunc free_value);

/**
 *\param table a HashTable
 *
 * Destroys all keys and values in the HashTable. Frees also memory allocated for the HashTable instance.
 */
void hashtable_destroy(HashTable *table);

/**
 *\param table a HashTable
 *
 * Destroys all keys and values in the HashTable.
 */
void hashtable_free(HashTable *table);

/**
 *\param table a HashTable
 *
 * Removes all elements from the HashTable.
 */
void hashtable_clear(HashTable *table);

/**
 *\param table a HashTable
 *\param key a key to insert
 *\param value the value to associate with the key
 *\param overwrite_key true to overwrite already exisiting keys
 *
 * Inserts a new key and value in the HashTable. If overwrite_key has been set an exisiting key will be
 * freed using the specified free_key function and then get replaced.
 */
void hashtable_set(HashTable *table, void * restrict key, void * restrict value, bool overwrite_key);

/**
 *\param table a HashTable
 *\param key a key
 *
 * Removes an element from the HashTable.
 */
void hashtable_remove(HashTable *table, const void *key);

/**
 *\param table a HashTable
 *\param key a key
 *\return a key
 *
 * Looks up a key in the HashTable and returns its value.
 */
void *hashtable_lookup(HashTable *table, const void *key);

/**
 *\param table a HashTable
 *\param key a key
 *\return true if given key does exist
 *
 * Checks if a key does exist.
 */
bool hashtable_key_exists(HashTable *table, const void *key);

/**
 *\param table a HashTable
 *\return number of stored elements
 *
 * Gets the number of stored elements.
 */
size_t hashtable_count(HashTable *table);

/**
 *\param table a HashTable
 *\param iter an uninitialized HashTableIter
 *
 * Initializes a key/value pair iterator and associates it with the table. Modifying the table while
 * using the iterator might lead to undefined behaviour.
 */
void hashtable_iter_init(HashTable *table, HashTableIter *iter);

/**
 *\param iter a HashTableIter
 *\return false if end of the HashTable has been reached
 *
 * Goes to next element of Hashtable.
 */
bool hashtable_iter_next(HashTableIter *iter);

/**
 *\param iter a HashTableIter
 *\return key of current element
 *
 * Retrieves the key of the current element.
 */
void *hashtable_iter_get_key(HashTableIter *iter);

/**
 *\param iter a HashTableIter
 *\return value of current element
 *
 * Retrieves the value of the current element.
 */
void *hashtable_iter_get_value(HashTableIter *iter);

#endif

