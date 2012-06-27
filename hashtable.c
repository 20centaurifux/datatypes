/***************************************************************************
    begin........: May 2012
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
 * \file hashtable.c
 * \brief A generic hashtable.
 * \author Sebastian Fedrau <lord-kefir@arcor.de>
 * \version 0.1.0
 * \date 26. June 2012
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifdef OPENMP
#include <omp.h>
#endif

#include "hashtable.h"

#define HASHTABLE_BUCKET_ALLOCATOR_BLOCK_SIZE 5192
#define HASHTABLE_LIST_ALLOCATOR_BLOCK_SIZE   512

/*
 *	public:
 */
#define HASHTABLE_INDEX(table, key) table->hash(key) % table->size

uint32_t inline
str_hash(const char *plain)
{
	uint32_t hash = 0;

	while(*plain)
	{
		hash = *plain++ + (hash << 6) + (hash << 16) - hash;
	}

	return hash;
}

HashTable *
hashtable_new(int32_t size, HashFunc hash_func, EqualFunc compare_keys, FreeFunc free_key, FreeFunc free_value)
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
hashtable_init(HashTable *table, int32_t size, HashFunc hash_func, EqualFunc compare_keys, FreeFunc free_key, FreeFunc free_value)
{
	assert(table != NULL);
	assert(size > 0);
	assert(size < INT32_MAX - 1);
	assert(hash_func != NULL);
	assert(compare_keys != NULL);

	if(!(table->buckets = (struct _Bucket **)calloc(size, sizeof(struct _Bucket *))))
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		abort();
	}

	table->allocator = (Allocator *)chunk_allocator_new(sizeof(struct _Bucket), HASHTABLE_LIST_ALLOCATOR_BLOCK_SIZE);

	table->compare_keys = compare_keys;
	table->free_key = free_key;
	table->free_value = free_value;
	table->size = size;
	table->hash = hash_func;
	table->count = 0;
}

void
hashtable_destroy(HashTable *table)
{
	hashtable_free(table);
	free(table);
}

void
hashtable_free(HashTable *table)
{
	int i;
	struct _Bucket *iter;

	if(table->free_key || table->free_value)
	{
		#ifdef OPENMP
		#pragma omp parallel for private(iter)
		#endif
		for(i = 0; i < table->size; ++i)
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
		#pragma omp barrier
	}

	chunk_allocator_destroy((ChunkAllocator *)table->allocator);
	free(table->buckets);
}

void
hashtable_clear(HashTable *table)
{
	int i;
	struct _Bucket *iter;

	if(!table->free_key && !table->free_value)
	{
		memset(table->buckets, 0, sizeof(struct _Bucket *) * table->size);
	}
	else
	{
		#ifdef OPENMP
		#pragma omp parallel for private(iter)
		#endif
		for(i = 0; i < table->size; ++i)
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
		#pragma omp barrier
	}

	chunk_allocator_destroy((ChunkAllocator *)table->allocator);
	table->allocator = (Allocator *)chunk_allocator_new(sizeof(struct _Bucket), HASHTABLE_LIST_ALLOCATOR_BLOCK_SIZE);

	table->count = 0;
}

void
hashtable_set(HashTable *table, void * restrict key, void * restrict value, bool overwrite_key)
{
	uint32_t index;
	struct _Bucket *head = NULL;
	struct _Bucket *iter;
	struct _Bucket *item = NULL;

	assert(table != NULL);
	assert(key != NULL);

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
		item = (struct _Bucket *)table->allocator->alloc(table->allocator);
		item->key = key;
		item->data = value;
		table->buckets[index] = item;
		item->next = head;
		table->count++;
	}
}

void
hashtable_remove(HashTable *table, const void *key)
{
	struct _Bucket *bucket;
	struct _Bucket *iter;
	struct _Bucket *prev = NULL;
	int32_t index;

	assert(table != NULL);
	assert(key != NULL);

	index = HASHTABLE_INDEX(table, key);

	if((iter = bucket = table->buckets[index]))
	{
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

				table->allocator->free(table->allocator, iter);
				table->count--;

				break;
			}

			prev = iter;
			iter = iter->next;
		}
	}
}

void *
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
				return iter->data;
			}

			iter = iter->next;
		}
	}

	return NULL;
}

bool
hashtable_key_exists(HashTable *table, const void *key)
{
	struct _Bucket *iter;

	if((iter = table->buckets[HASHTABLE_INDEX(table, key)]))
	{
		while(iter)
		{
			if(table->compare_keys(key, iter->key))
			{
				return true;
			}
		}
	}

	return false;
}

inline uint32_t
hashtable_count(HashTable *table)
{
	assert(table != NULL);

	return table->count;
}

void
hashtable_iter_init(HashTable *table, HashTableIter *iter)
{
	memset(iter, 0, sizeof(HashTableIter));
	iter->table = table;
}

static inline bool
_hashtable_iter_get_next_bucket(HashTableIter *iter)
{
	iter->liter = NULL;

	while(!iter->liter && iter->offset < iter->table->size)
	{
		iter->liter = iter->table->buckets[iter->offset++];
	}

	return iter->liter ? true : false;
}

bool
hashtable_iter_next(HashTableIter *iter)
{
	if(iter->finished)
	{
		return false;
	}

	for( ;; )
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

void inline *
hashtable_iter_get_key(HashTableIter *iter)
{
	assert(iter != NULL);

	if(iter->liter)
	{
		return iter->liter->key;
	}

	return false;
}

void inline *
hashtable_iter_get_value(HashTableIter *iter)
{
	assert(iter != NULL);

	if(iter->liter)
	{
		return iter->liter->data;
	}

	return false;
}

