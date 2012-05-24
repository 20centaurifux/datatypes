#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

#include <stdbool.h>

#include "generic.h"
#include "rbtree.h"

typedef uint32_t (*HashFunc)(const char *plain);

typedef struct
{
	CompareFunc compare_keys;
	FreeFunc free_key;
	FreeFunc free_value;
	HashFunc hash;
	uint32_t size;
	RBTree **buckets;
	RBTree *pool;
	RBTree *poolptr;
	uint32_t count;
	Allocator *allocator;
} HashTable;

uint32_t str_hash(const char *plain);

HashTable *hashtable_new(uint32_t size, HashFunc hash_func, CompareFunc compare_keys, FreeFunc free_key, FreeFunc free_value);
void hashtable_destroy(HashTable *table);
void hashtable_set(HashTable *table, void * restrict key, void * restrict value, bool overwrite_key);
void hashtable_remove(HashTable *table, const void *key);
void *hashtable_lookup(HashTable *table, const void *key);
bool hashtable_key_exists(HashTable *table, const void *key);
#define hashtable_count(table) table->count
bool hashtable_foreach(HashTable *table, ForeachKeyValuePairFunc foreach, void *user_data);

#endif

