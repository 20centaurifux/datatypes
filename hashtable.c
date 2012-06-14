#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifdef PTHREADS
#include <pthread.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif

#include "hashtable.h"
#include "rbtree.h"

#define HASHTABLE_BUCKET_ALLOCATOR_BLOCK_SIZE 5192

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
	int block_div = 1;

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

	if(table->size >= 8)
	{
		block_div = 8;
	}

	table->list_allocator = (Allocator *)g_allocator_new(sizeof(ListItem), table->size / block_div);
}

#if defined(PTHREADS) || defined(WIN32)
static void *
_hashtable_destroy_worker(void *arg)
{
	HashTable *table = (HashTable *)arg;
	int i;
	int from;

	from = table->size / 2 + 1;

	for(i = from ; i < table->size; ++i)
	{
		if(table->buckets[i])
		{
			list_free(table->buckets[i]);
		}
	}

	#ifdef PTHREADS
	pthread_exit(NULL);
	#else
	return NULL;
	#endif
}
#endif

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
	int to;

	#if defined(PTHREADS)
	pthread_t thread;
	pthread_attr_t attr;
	#elif defined(WIN32)
	HANDLE thread = NULL;
	#endif

	#if defined(PTHREADS) || defined(WIN32)
	if(table->size >= 256)
	{
		to = table->size / 2;

		#ifdef PTHREADS
		pthread_attr_init(&attr);
		pthread_create(&thread, &attr, _hashtable_destroy_worker, table);
		#else
		thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)_hashtable_destroy_worker, table, 0, NULL);
		#endif
	}
	else
	{
		to = table->size;
	}
	#else
	to = table->size;
	#endif

	for(i = 0; i < to; ++i)
	{
		if(table->buckets[i])
		{
			list_free(table->buckets[i]);
		}
	}

	#if defined(PTHREADS)
	if(table->size >= 256)
	{
		pthread_join(thread, NULL);
		pthread_detach(thread);
		pthread_attr_destroy(&attr);
	}
	#elif defined(WIN32)
	if(thread)
	{
		WaitForSingleObject(thread, INFINITE);
	}
	#endif

	g_allocator_destroy((GAllocator *)table->allocator);
	g_allocator_destroy((GAllocator *)table->list_allocator);
	free(table->pool);
}

#if defined(PTHREADS) || defined(WIN32)
static void *
_hashtable_clear_worker(void *arg)
{
	HashTable *table = (HashTable *)arg;
	int i;
	int from;

	from = table->size / 2 + 1;

	for(i = from ; i < table->size; ++i)
	{
		if(table->buckets[i])
		{
			list_clear(table->buckets[i]);
			table->buckets[i] = NULL;
		}
	}

	#ifdef PTHREADS
	pthread_exit(NULL);
	#else
	return NULL;
	#endif
}

#endif
void
hashtable_clear(HashTable *table)
{
	int i;
	int to;

	#if defined(PTHREADS)
	pthread_t thread;
	pthread_attr_t attr;
	#elif defined(WIN32)
	HANDLE thread = NULL;
	#endif

	#if defined(PTHREADS) || defined(WIN32)
	if(table->size >= 256)
	{
		to = table->size / 2;

		#ifdef PTHREADS
		pthread_attr_init(&attr);
		pthread_create(&thread, &attr, _hashtable_clear_worker, table);
		#else
		thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)_hashtable_clear_worker, table, 0, NULL);
		#endif
	}
	else
	{
		to = table->size;
	}
	#else
	to = table->size;
	#endif

	for(i = 0; i < to; ++i)
	{
		if(table->buckets[i])
		{
			list_clear(table->buckets[i]);
			table->buckets[i] = NULL;
		}
	}

	#if defined(PTHREADS)
	if(table->size >= 256)
	{
		pthread_join(thread, NULL);
		pthread_detach(thread);
		pthread_attr_destroy(&attr);
	}
	#elif defined(WIN32)
	if(thread)
	{
		WaitForSingleObject(thread, INFINITE);
	}
	#endif

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
		list_init(bucket, _hashtable_item_equals, table->free_key, table->list_allocator);
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
				table->free_key(item->value);
			}

			table->allocator->free(table->allocator, item);
			list_item_set_data(el, NULL);

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

