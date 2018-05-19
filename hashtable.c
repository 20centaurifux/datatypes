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
 * \file hashtable.c
 * \brief A generic hashtable.
 * \author Sebastian Fedrau <sebastian.fedrau@gmail.com>
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "hashtable.h"

/*! Memory pool block size. */
#define HASHTABLE_BUCKET_POOL_BLOCK_SIZE 64

/*! Initial hashtable size when resizing automatically. */
#define HASHTABLE_INITIAL_SIZE           256

/**
 *\param table a HashTable
 *\param key a key
 *\return index of the given key
 *
 * Calculates table index of a key.
 */
#define HASHTABLE_INDEX(table, key) table->hash(key) % table->size

HashTable *
hashtable_new(size_t size, HashFunc hash_func, EqualFunc compare_keys, FreeFunc free_key, FreeFunc free_value)
{
	HashTable *table;

	if(!(table = (HashTable *)malloc(sizeof(HashTable))))
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		abort();
	}

	hashtable_init(table, size, hash_func, compare_keys, free_key, free_value);

	return table;
}

inline void
hashtable_init(HashTable *table, size_t size, HashFunc hash_func, EqualFunc compare_keys, FreeFunc free_key, FreeFunc free_value)
{
	size_t table_size = HASHTABLE_INITIAL_SIZE;

	assert(table != NULL);
	assert(hash_func != NULL);
	assert(compare_keys != NULL);

	if(size != HASHTABLE_AUTO_RESIZE)
	{
		table_size = size;
	}

	if(!(table->buckets = (struct _Bucket **)calloc(table_size, sizeof(struct _Bucket *))))
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		abort();
	}

	table->pool = (Pool *)memory_pool_new(sizeof(struct _Bucket), HASHTABLE_BUCKET_POOL_BLOCK_SIZE);

	table->compare_keys = compare_keys;
	table->free_key = free_key;
	table->free_value = free_value;
	table->size = table_size;
	table->grow = size == HASHTABLE_AUTO_RESIZE;
	table->hash = hash_func;
	table->count = 0;
	table->pair.free_value = free_value;
	table->pair.bucket = NULL;
}

void
hashtable_destroy(HashTable *table)
{
	assert(table != NULL);

	hashtable_free(table);
	free(table);
}

void
hashtable_free(HashTable *table)
{
	struct _Bucket *iter;

	assert(table != NULL);

	if(table->free_key || table->free_value)
	{
		for(size_t i = 0; i < table->size; ++i)
		{
			if((iter = table->buckets[i]))
			{
				while(iter)
				{
					if(table->free_key)
					{
						table->free_key(iter->key);
					}

					if(table->free_value)
					{
						table->free_value(iter->data);
					}

					iter = iter->next;
				}
			}
		}
	}

	memory_pool_destroy((MemoryPool *)table->pool);
	free(table->buckets);
}

void
hashtable_clear(HashTable *table)
{
	struct _Bucket *iter;

	assert(table != NULL);

	if(!table->free_key && !table->free_value)
	{
		memset(table->buckets, 0, sizeof(struct _Bucket *) * table->size);
	}
	else
	{
		for(size_t i = 0; i < table->size; ++i)
		{
			if((iter = table->buckets[i]))
			{
				while(iter)
				{
					if(table->free_key)
					{
						table->free_key(iter->key);
					}

					if(table->free_value)
					{
						table->free_value(iter->data);
					}

					iter = iter->next;
				}
			}

			table->buckets[i] = NULL;
		}
	}

	memory_pool_destroy((MemoryPool *)table->pool);
	table->pool = (Pool *)memory_pool_new(sizeof(struct _Bucket), HASHTABLE_BUCKET_POOL_BLOCK_SIZE);

	table->count = 0;
}

static void
_hashtable_resize(HashTable *table)
{
	size_t old_size;
	struct _Bucket **buckets;

	assert(table != NULL);

	/* resize table */
	old_size = table->size;
	table->size *= (table->size >= 0x400000) ? 2 : 4;

	if(old_size > table->size)
	{
		fprintf(stderr, "%s: integer overflow.\n", __func__);
		abort();
	}

	/* create new bucket list */
	buckets = table->buckets;

	if(!(table->buckets = (struct _Bucket **)calloc(table->size, sizeof(struct _Bucket *))))
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		abort();
	}

	/* copy items to new bucket array */
	for(size_t i = 0; i < old_size; ++i)
	{
		struct _Bucket *iter = buckets[i];

		while(iter)
		{
			struct _Bucket *next = iter->next;

			size_t index = HASHTABLE_INDEX(table, iter->key);

			if(table->buckets[index])
			{
				iter->next = table->buckets[index];
				table->buckets[index] = iter;
			}
			else
			{
				iter->next = NULL;
				table->buckets[index] = iter;
			}

			iter = next;
		}
	}

	free(buckets);
}

void
hashtable_set(HashTable *table, void * restrict key, void * restrict value, bool overwrite_key)
{
	size_t index;
	struct _Bucket *head = NULL;
	struct _Bucket *iter;
	struct _Bucket *item = NULL;

	assert(table != NULL);
	assert(key != NULL);

	/* resize table */
	if(table->grow && table->size == table->count)
	{
		_hashtable_resize(table);
	}

	index = HASHTABLE_INDEX(table, key);
	assert(index < table->size);

	if((iter = head = table->buckets[index]))
	{
		/* found bucket => try to find list item with given key */
		while(iter)
		{
			if(table->compare_keys(key, iter->key))
			{
				item = iter;
				break;
			}

			iter = iter->next;
		}
	}

	if(item)
	{
		/* overwrite existing item */
		if(overwrite_key)
		{
			if(table->free_key)
			{
				table->free_key(item->key);
			}

			item->key = key;
		}

		if(table->free_value)
		{
			table->free_value(item->data);
		}

		item->data = value;
	}
	else
	{
		/* insert new item */
		item = (struct _Bucket *)table->pool->alloc(table->pool);
		item->key = key;
		item->data = value;
		table->buckets[index] = item;
		item->next = head;
		++table->count;
	}
}

void
hashtable_remove(HashTable *table, const void *key)
{
	struct _Bucket *bucket;
	struct _Bucket *iter;
	int32_t index;

	assert(table != NULL);
	assert(key != NULL);

	index = HASHTABLE_INDEX(table, key);

	if((iter = bucket = table->buckets[index]))
	{
		struct _Bucket *prev = NULL;

		while(iter)
		{
			if(table->compare_keys(iter->key, key))
			{
				if(table->free_key)
				{
					table->free_key(iter->key);
				}

				if(table->free_value)
				{
					table->free_value(iter->data);
				}

				if(prev)
				{
					prev->next = iter->next;
				}
				else
				{
					table->buckets[index] = iter->next;
				}

				table->pool->free(table->pool, iter);
				--table->count;

				break;
			}

			prev = iter;
			iter = iter->next;
		}
	}
}

HashTablePair *
hashtable_lookup(HashTable *table, const void *key)
{
	struct _Bucket *iter;

	assert(table != NULL);
	assert(key != NULL);

	if((iter = table->buckets[HASHTABLE_INDEX(table, key)]))
	{
		while(iter)
		{
			if(table->compare_keys(iter->key, key))
			{
				table->pair.bucket = iter;

				return &table->pair;
			}

			iter = iter->next;
		}
	}

	return NULL;
}

void *
hashtable_pair_get_key(const HashTablePair *pair)
{
	assert(pair != NULL);
	assert(pair->bucket != NULL);
	assert(pair->bucket->key != NULL);

	if(pair)
	{
		return pair->bucket->key;
	}

	return NULL;
}

void *
hashtable_pair_get_value(const HashTablePair *pair)
{
	assert(pair != NULL);
	assert(pair->bucket != NULL);
	assert(pair->bucket->key != NULL);

	if(pair)
	{
		return pair->bucket->data;
	}

	return NULL;
}

void
hashtable_pair_set_value(HashTablePair *pair, void *value)
{
	assert(pair != NULL);
	assert(pair->bucket != NULL);
	assert(pair->bucket->key != NULL);

	if(pair->free_value && pair->bucket->data)
	{
		pair->free_value(pair->bucket->data);
	}

	pair->bucket->data = value;
}

bool
hashtable_key_exists(const HashTable *table, const void *key)
{
	struct _Bucket *iter;

	assert(table != NULL);
	assert(key != NULL);

	if((iter = table->buckets[HASHTABLE_INDEX(table, key)]))
	{
		while(iter)
		{
			if(table->compare_keys(key, iter->key))
			{
				return true;
			}

			iter = iter->next;
		}
	}

	return false;
}

inline size_t
hashtable_count(const HashTable *table)
{
	assert(table != NULL);

	return table->count;
}

void
hashtable_iter_init(const HashTable *table, HashTableIter *iter)
{
	assert(table != NULL);

	memset(iter, 0, sizeof(HashTableIter));
	iter->table = table;
}

static inline bool
_hashtable_iter_get_next_bucket(HashTableIter *iter)
{
	assert(iter != NULL);

	iter->liter = NULL;

	while(!iter->liter && iter->offset < iter->table->size)
	{
		iter->liter = iter->table->buckets[iter->offset++];
	}

	return iter->liter != NULL;
}

bool
hashtable_iter_next(HashTableIter *iter)
{
	assert(iter != NULL);

	if(iter->finished)
	{
		return false;
	}

	for(;;)
	{
		if(iter->liter)
		{
			if((iter->liter = iter->liter->next))
			{
				return true;
			}
		}
		else
		{
			if(!_hashtable_iter_get_next_bucket(iter))
			{
				iter->finished = true;

				return false;
			}
			else
			{
				return true;
			}
		}
	}
}

inline void *
hashtable_iter_get_key(const HashTableIter *iter)
{
	assert(iter != NULL);

	if(iter->liter)
	{
		return iter->liter->key;
	}

	return NULL;
}

inline void *
hashtable_iter_get_value(const HashTableIter *iter)
{
	assert(iter != NULL);

	if(iter->liter)
	{
		return iter->liter->data;
	}

	return NULL;
}

