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
 * \brief Generic hashtable.
 * \author Sebastian Fedrau <sebastian.fedrau@gmail.com>
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "hashtable.h"

/*! @cond INTERNAL */

/* Memory pool block size. */
#define HASHTABLE_BUCKET_POOL_BLOCK_SIZE 128

/* Initial hashtable size when resizing automatically. */
#define HASHTABLE_INITIAL_SIZE           257

/* Prime numbers used for table size. */
static const size_t PRIMES[] =
{
	257,
	1031,
	4127,
	16519,
	66083,
	264343,
	1057387,
	4229573,
	8459173,
	16918357,
	33836729,
	67673503,
	135347029,
	270694063,
	541388129,
	1082776297,
	2165552617,
	#if SIZE_MAX == 18446744073709551615UL
	4331105239,
	8662210517,
	17324420957,
	34648841923,
	69297683827,
	138595367657,
	277190735317,
	554381470637,
	110876294129,
	221752588259,
	443505176519,
	887010353059,
	1774020706189,
	3548041412411,
	7096082824837,
	14192165649709,
	28384331299451,
	56768662598903,
	113537325197831,
	227074650395669,
	454149300791371,
	908298601582769,
	1816597203165593,
	3633194406331187,
	7266388812662387,
	14532777625324787,
	29065555250649649,
	58131110501299349,
	116262221002598761,
	232524442005197556,
	465048884010395089,
	930097768020790181,
	#endif
	0
};

/* Calculates table index of a key. */
#define HASHTABLE_INDEX(table, key) table->hash(key) % table->size

/*! @endcond */

HashTable *
hashtable_new(size_t size, HashFunc hash_func, EqualFunc compare_keys, FreeFunc free_key, FreeFunc free_value)
{
	assert(hash_func != NULL);
	assert(compare_keys != NULL);
	assert(size <= SIZE_MAX / sizeof(struct _Bucket *));

	HashTable *table = (HashTable *)malloc(sizeof(HashTable));

	if(!table)
	{
		perror("malloc()");
		abort();
	}

	hashtable_init(table, size, hash_func, compare_keys, free_key, free_value);

	return table;
}

void
hashtable_init(HashTable *table, size_t size, HashFunc hash_func, EqualFunc compare_keys, FreeFunc free_key, FreeFunc free_value)
{
	assert(table != NULL);
	assert(hash_func != NULL);
	assert(compare_keys != NULL);
	assert(size <= SIZE_MAX / sizeof(struct _Bucket *));

	size_t table_size = HASHTABLE_INITIAL_SIZE;

	if(size != HASHTABLE_AUTO_RESIZE)
	{
		table_size = size;
	}

	table->buckets = (struct _Bucket **)calloc(table_size, sizeof(struct _Bucket *));

	if(!table->buckets)
	{
		perror("calloc()");
		abort();
	}

	table->pool = (Pool *)memory_pool_new(sizeof(struct _Bucket), HASHTABLE_BUCKET_POOL_BLOCK_SIZE);

	table->compare_keys = compare_keys;
	table->free_key = free_key;
	table->free_value = free_value;
	table->size = table_size;

	if(size == HASHTABLE_AUTO_RESIZE)
	{
		table->grow = true;
		table->sizeptr = PRIMES;
	}
	else
	{
		table->grow = false;
		table->sizeptr = NULL;
	}

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

static void
_hashtable_free_memory(HashTable *table)
{
	assert(table != NULL);

	if(table->free_key || table->free_value)
	{
		for(size_t i = 0; i < table->size; ++i)
		{
			struct _Bucket *iter;

			if((iter = table->buckets[i]))
			{
				while(iter)
				{
					if(table->free_key)
					{
						table->free_key(iter->key);
					}

					if(table->free_value && iter->data)
					{
						table->free_value(iter->data);
					}

					iter = iter->next;
				}
			}
		}
	}
}

void
hashtable_free(HashTable *table)
{
	assert(table != NULL);

	_hashtable_free_memory(table);
	memory_pool_destroy((MemoryPool *)table->pool);
	free(table->buckets);
}

void
hashtable_clear(HashTable *table)
{
	assert(table != NULL);

	table->count = 0;

	_hashtable_free_memory(table);
	memset(table->buckets, 0, sizeof(struct _Bucket *) * table->size);

	memory_pool_destroy((MemoryPool *)table->pool);
	table->pool = (Pool *)memory_pool_new(sizeof(struct _Bucket), HASHTABLE_BUCKET_POOL_BLOCK_SIZE);
}

static void
_hashtable_resize(HashTable *table)
{
	assert(table != NULL);
	assert(table->size > 0);

	size_t old_size = table->size;

	table->sizeptr++;
	table->size = *table->sizeptr;

	if(!table->size && table->size <= SIZE_MAX / sizeof(struct _Bucket *))
	{
		fprintf(stderr, "%s(): integer overflow.\n", __func__);
		abort();
	}

	struct _Bucket **buckets = table->buckets;

	table->buckets = (struct _Bucket **)calloc(table->size, sizeof(struct _Bucket *));

	if(!table->buckets)
	{
		perror("calloc()");
		abort();
	}

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

static struct _Bucket *
_hashtable_find_bucket(const HashTable *table, struct _Bucket *head, const void *key)
{
	assert(table != NULL);
	assert(key != NULL);

	struct _Bucket *iter = head;
	struct _Bucket *item = NULL;

	while(iter && !item)
	{
		if(table->compare_keys(key, iter->key))
		{
			item = iter;
		}

		iter = iter->next;
	}

	return item;
}

HashTableInsertResult
hashtable_set(HashTable *table, void *key, void *value, bool overwrite_key)
{
	HashTableInsertResult result = HASHTABLE_INSERT_RESULT_FAILED;

	assert(table != NULL);
	assert(key != NULL);

	if(table->grow && table->count > table->size)
	{
		_hashtable_resize(table);
	}

	size_t index = HASHTABLE_INDEX(table, key);
	struct _Bucket *head = table->buckets[index];
	struct _Bucket *item = _hashtable_find_bucket(table, head, key);

	if(item)
	{
		result = HASHTABLE_INSERT_RESULT_REPLACED;

		if(overwrite_key)
		{
			if(table->free_key)
			{
				table->free_key(item->key);
			}

			item->key = key;
		}

		if(table->free_value && item->data)
		{
			table->free_value(item->data);
		}

		item->data = value;
	}
	else if(table->count < SIZE_MAX)
	{
		result = HASHTABLE_INSERT_RESULT_NEW;

		item = (struct _Bucket *)table->pool->alloc(table->pool);

		item->key = key;
		item->data = value;
		table->buckets[index] = item;
		item->next = head;

		++table->count;
	}
	else
	{
		fprintf(stderr, "%s(): integer overflow.\n", __func__);
	}

	return result;
}

void
hashtable_remove(HashTable *table, const void *key)
{
	assert(table != NULL);
	assert(key != NULL);

	struct _Bucket *bucket;
	struct _Bucket *iter;
	size_t index = HASHTABLE_INDEX(table, key);

	if((iter = bucket = table->buckets[index]))
	{
		struct _Bucket *prev = NULL;
		bool removed = false;

		while(iter && !removed)
		{
			if(table->compare_keys(iter->key, key))
			{
				if(table->free_key)
				{
					table->free_key(iter->key);
				}

				if(table->free_value && iter->data)
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

				removed = true;
			}

			prev = iter;
			iter = iter->next;
		}
	}
}

HashTablePair *
hashtable_lookup(HashTable *table, const void *key)
{
	assert(table != NULL);
	assert(key != NULL);

	struct _Bucket *head = table->buckets[HASHTABLE_INDEX(table, key)];
	struct _Bucket *item = _hashtable_find_bucket(table, head, key);

	if(item)
	{
		table->pair.bucket = item;

		return &table->pair;
	}

	return NULL;
}

void *
hashtable_pair_get_key(const HashTablePair *pair)
{
	assert(pair != NULL);
	assert(pair->bucket != NULL);
	assert(pair->bucket->key != NULL);

	return pair->bucket->key;
}

void *
hashtable_pair_get_value(const HashTablePair *pair)
{
	assert(pair != NULL);
	assert(pair->bucket != NULL);
	assert(pair->bucket->key != NULL);

	return pair->bucket->data;
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
	assert(table != NULL);
	assert(key != NULL);

	struct _Bucket *head = table->buckets[HASHTABLE_INDEX(table, key)];

	return _hashtable_find_bucket(table, head, key) != NULL;
}

size_t
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

static bool
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

	bool success = false;

	while(!success && !iter->finished)
	{
		if(iter->liter)
		{
			iter->liter = iter->liter->next;
			success = iter->liter != NULL;
		}
		else
		{
			success = _hashtable_iter_get_next_bucket(iter);
			iter->finished = !success;
		}
	}

	return success;
}

void *
hashtable_iter_get_key(const HashTableIter *iter)
{
	assert(iter != NULL);

	return iter->liter->key;
}

void *
hashtable_iter_get_value(const HashTableIter *iter)
{
	assert(iter != NULL);

	return iter->liter->data;
}

