#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <omp.h>

#include "hashtable.h"
#include "list.h"

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

	if(!(table->buckets = (List **)calloc(size, sizeof(List *))))
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		abort();
	}

	if(!(table->pool = (List *)malloc(size * sizeof(List))))
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		abort();
	}

	table->compare_keys = compare_keys;
	table->free_key = free_key;
	table->free_value = free_value;
	table->size = size;
	table->hash = hash_func;
	table->poolptr = table->pool;
	table->count = 0;

	table->allocator = (Allocator *)g_allocator_new(sizeof(_HashtableItem), HASHTABLE_BUCKET_ALLOCATOR_BLOCK_SIZE);
	table->list_allocator = (Allocator *)g_allocator_new(sizeof(ListItem), HASHTABLE_LIST_ALLOCATOR_BLOCK_SIZE);
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
	ListItem *iter;

	if(table->free_key || table->free_value)
	{
		#pragma omp parallel for private(iter)
		for(i = 0; i < table->size; ++i)
		{
			if(table->buckets[i])
			{
				iter = list_head(table->buckets[i]);

				while(iter)
				{
					if(table->free_key)
					{
						table->free_key(((_HashtableItem *)list_item_get_data(iter))->key);
					}

					if(table->free_value)
					{
						table->free_value(((_HashtableItem *)list_item_get_data(iter))->value);
					}

					iter = list_item_next(iter);
				}
			}
		}
		#pragma omp barrier
	}

	g_allocator_destroy((GAllocator *)table->allocator);
	g_allocator_destroy((GAllocator *)table->list_allocator);
	free(table->buckets);
	free(table->pool);
}

void
hashtable_clear(HashTable *table)
{
	int i;
	ListItem *iter;

	if(!table->free_key && !table->free_value)
	{
		#pragma omp parallel for
		for(i = 0; i < table->size; ++i)
		{
			table->buckets[i] = NULL;
		}
		#pragma omp barrier
	}
	else
	{
		#pragma omp parallel for private(iter)
		for(i = 0; i < table->size; ++i)
		{
			if(table->buckets[i])
			{
				iter = list_head(table->buckets[i]);

				while(iter)
				{
					if(table->free_key)
					{
						table->free_key(((_HashtableItem *)list_item_get_data(iter))->key);
					}

					if(table->free_value)
					{
						table->free_value(((_HashtableItem *)list_item_get_data(iter))->value);
					}

					iter = list_item_next(iter);
				}
			}

			table->buckets[i] = NULL;
		}
		#pragma omp barrier
	}

	g_allocator_destroy((GAllocator *)table->list_allocator);
	table->list_allocator = (Allocator *)g_allocator_new(sizeof(ListItem), HASHTABLE_LIST_ALLOCATOR_BLOCK_SIZE);

	table->poolptr = table->pool;
	table->count = 0;
}

static bool
_hashtable_item_equals(void const *a, void const *b)
{
	_HashtableItem *item = (_HashtableItem *)a;
	const char *key = (const char *)b;

	return str_equal(item->key, key);
}

void
hashtable_set(HashTable *table, void * restrict key, void * restrict value, bool overwrite_key)
{
	uint32_t index;
	List *bucket;
	ListItem *el;
	_HashtableItem *item = NULL;

	assert(table != NULL);
	assert(key != NULL);

	index = HASHTABLE_INDEX(table, key);
	assert(index < table->size);

	if(!(bucket = table->buckets[index]))
	{
		bucket = table->buckets[index] = table->poolptr++;
		list_init(bucket, _hashtable_item_equals, NULL, table->list_allocator);
	}

	if((el = list_find(bucket, NULL, key)))
	{
		item = list_item_get_data(el);

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
			table->free_value(item->value);
		}

		item->value = value;
	}
	else
	{
		item = (_HashtableItem *)table->allocator->alloc(table->allocator);
		item->key = key;
		item->value = value;
		list_prepend(bucket, item);
		table->count++;
	}
}

void
hashtable_remove(HashTable *table, const void *key)
{
	List *bucket;
	ListItem *el;
	_HashtableItem *item;

	assert(table != NULL);
	assert(key != NULL);

	if((bucket = table->buckets[HASHTABLE_INDEX(table, key)]))
	{
		if((el = list_find(bucket, NULL, key)))
		{
			item = list_item_get_data(el);

			if(table->free_key)
			{
				table->free_key(item->key);
			}

			if(table->free_value)
			{
				table->free_value(item->value);
			}

			list_item_set_data(el, NULL);
			
			table->allocator->free(table->allocator, item);

			list_remove(bucket, el);
			table->count--;
		}
	}
}

void *
hashtable_lookup(HashTable *table, const void *key)
{
	List *bucket;
	ListItem *el;
	_HashtableItem *item;

	assert(table != NULL);
	assert(key != NULL);

	if((bucket = table->buckets[HASHTABLE_INDEX(table, key)]))
	{
		if((el = list_find(bucket, NULL, key)))
		{
			item = list_item_get_data(el);

			if(table->compare_keys(key, item->key))
			{
				return item->value;
			}
		}
	}

	return NULL;
}

bool
hashtable_key_exists(HashTable *table, const void *key)
{
	List *bucket;

	if((bucket = table->buckets[HASHTABLE_INDEX(table, key)]))
	{
		if((list_find(bucket, NULL, key)))
		{
			return true;
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
		if(iter->table->buckets[iter->offset])
		{
			iter->liter = list_head(iter->table->buckets[iter->offset]);
		}

		iter->offset++;
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
			if((iter->liter = list_item_next(iter->liter)))
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
		return ((_HashtableItem *)list_item_get_data(iter->liter))->key;
	}

	return false;
}

void inline *
hashtable_iter_get_value(HashTableIter *iter)
{
	assert(iter != NULL);

	if(iter->liter)
	{
		return ((_HashtableItem *)list_item_get_data(iter->liter))->value;
	}

	return false;
}

