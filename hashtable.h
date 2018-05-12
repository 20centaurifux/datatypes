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
 * \file hashtable.h
 * \brief A generic hashtable.
 * \author Sebastian Fedrau <sebastian.fedrau@gmail.com>
 */
#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdbool.h>

#include "datatypes.h"
#include "pool.h"

/*! HashTable size used to enable auto-resizing. */
#define HASHTABLE_AUTO_RESIZE 0

/**
 *\struct HashTable
 *\brief A datatype to create associations between keys and values.
 */
typedef struct _HashTable
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
	/*! Enable/disable auto-resize. */
	bool grow;
	/**
	 *\struct _Bucket
	 *\brief A singly-linked list implementation storing keys & values.
	 *
	 *\var buckets
	 *\brief An array containing pointers to buckets.
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
	/*! Pool used to create/destroy list elements. */
	Pool *pool;
	/*! A Found key-value pair. */
	struct _HashTablePair
	{
		/*! A function to free the associated value. */
		FreeFunc free_value;
		/*! Bucket containing the found key-value pair. */
		struct _Bucket *bucket;
	} pair;
} HashTable;

/*! A found key-value pair. */
typedef struct _HashTablePair HashTablePair;

/**
 *\struct HashTableIter
 *\brief A structure to iterate over the elements of a HashTable.
 */
typedef struct
{
	/*! Pointer to the associated HashTable. */
	const HashTable *table;
	/*! Index of current bucket. */
	size_t offset;
	/*! Pointer to current list element. */
	struct _Bucket *liter;
		/*! true if iteration is completed. */
	bool finished;
} HashTableIter;

/**
 *\param ptr pointer to plain text
 *\return a hash value
 *
 * Calculates the hash of the specified plain text.
 */
uint32_t str_hash(const void *ptr);

/**
 *\param size size of the hash table (number of buckets), HASHTABLE_AUTO_RESIZE to grow automatically
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
 *\param size size of the hash table (number of buckets), HASHTABLE_AUTO_RESIZE to grow automatically
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
 * Frees all keys, values and the table pointer.
 */
void hashtable_destroy(HashTable *table);

/**
 *\param table a HashTable
 *
 * Frees all keys and values without freeing the table pointer.
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
 *\param overwrite_key true to overwrite already existing keys
 *
 * Inserts a new key and value in the HashTable. If overwrite_key is set an existing key is
 * freed using the specified free_key function before it gets replaced.
 */
void hashtable_set(HashTable *table, void * restrict key, void * restrict value, bool overwrite_key);

/**
 *\param table a HashTable
 *\param key key of the element to remove
 *
 * Removes an element from the HashTable.
 */
void hashtable_remove(HashTable *table, const void *key);

/**
 *\param table a HashTable
 *\param key a key
 *\return the found key-value pair or NULL
 *
 * Looks up a key-value pair in the HashTable.
 */
HashTablePair *hashtable_lookup(HashTable *table, const void *key);

/*! Returns a pointer to the key of a key-value pair. */
#define hashtable_pair_get_key(p) p->bucket->key

/*! Returns a pointer to the value of a key-value pair. */
#define hashtable_pair_get_value(p) p->bucket->data

/**
 *\param pair a HashTablePair
 *\param value new value to set
 *
 * Overwrites the value of a key-value pair.
 */
void hashtable_pair_set_value(HashTablePair *pair, void *value);

/**
 *\param table a HashTable
 *\param key a key
 *\return true if given key does exist
 *
 * Checks if a key does exist.
 */
bool hashtable_key_exists(const HashTable *table, const void *key);

/**
 *\param table a HashTable
 *\return number of stored elements
 *
 * Gets the number of stored elements.
 */
size_t hashtable_count(const HashTable *table);

/**
 *\param table a HashTable
 *\param iter an uninitialized HashTableIter
 *
 * Initializes a key/value pair iterator and associates it with the table. Modifying the table while
 * using the iterator might lead to undefined behaviour.
 */
void hashtable_iter_init(const HashTable *table, HashTableIter *iter);

/**
 *\param iter a HashTableIter
 *\return false if end of the HashTable has been reached
 *
 * Goes to next element of the HashTable.
 */
bool hashtable_iter_next(HashTableIter *iter);

/**
 *\param iter a HashTableIter
 *\return key of current element
 *
 * Retrieves the key of the current element.
 */
void *hashtable_iter_get_key(const HashTableIter *iter);

/**
 *\param iter a HashTableIter
 *\return value of current element
 *
 * Retrieves the value of the current element.
 */
void *hashtable_iter_get_value(const HashTableIter *iter);

#endif

