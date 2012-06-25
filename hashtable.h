#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

#include <stdbool.h>

#include "datatypes.h"
#include "allocator.h"

typedef uint32_t (*HashFunc)(const char *plain);

typedef struct
{
	void *key;
	void *value;
} _HashtableItem;

typedef struct
{
	EqualFunc compare_keys;
	FreeFunc free_key;
	FreeFunc free_value;
	HashFunc hash;
	int32_t size;
	struct _Bucket
	{
		void *key;
		void *data;
		void *next;
	} **buckets;
	uint32_t count;
	Allocator *allocator;
} HashTable;

typedef struct
{
	HashTable *table;
	uint32_t offset;
	struct _Bucket *liter;
	bool finished:1;
} HashTableIter;

uint32_t str_hash(const char *plain);

HashTable *hashtable_new(int32_t size, HashFunc hash_func, EqualFunc compare_keys, FreeFunc free_key, FreeFunc free_value);
void hashtable_init(HashTable *table, int32_t size, HashFunc hash_func, EqualFunc compare_keys, FreeFunc free_key, FreeFunc free_value);
void hashtable_destroy(HashTable *table);
void hashtable_free(HashTable *table);
void hashtable_clear(HashTable *table);
void hashtable_set(HashTable *table, void * restrict key, void * restrict value, bool overwrite_key);
void hashtable_remove(HashTable *table, const void *key);
void *hashtable_lookup(HashTable *table, const void *key);
bool hashtable_key_exists(HashTable *table, const void *key);
uint32_t hashtable_count(HashTable *table);
void hashtable_iter_init(HashTable *table, HashTableIter *iter);
bool hashtable_iter_next(HashTableIter *iter);
void *hashtable_iter_get_key(HashTableIter *iter);
void *hashtable_iter_get_value(HashTableIter *iter);

#endif

