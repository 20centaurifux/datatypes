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

/*
 *	node allocator:
 */
typedef struct
{
	Allocator padding;

	/* store blocks of memory in a list */
	struct _NodeBlock
	{
		RBNode *nodes;
		int offset;
		struct _NodeBlock *next;
	} *block;
	/* store free nodes in blocks containing pointers */
	struct _NodePtrBlock
	{
		RBNode **nodes;
		int offset;
		struct _NodePtrBlock *next;
		struct _NodePtrBlock *prev;
	} *free_block;
	bool reuse_nodes;
	int block_size;
} NodeAllocator;

static struct _NodeBlock *
_node_allocator_create_block(int block_size)
{
	struct _NodeBlock *block;

	if(!(block = (struct _NodeBlock *)malloc(sizeof(struct _NodeBlock))))
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		abort();
	}

	if(!(block->nodes = (RBNode *)malloc(sizeof(RBNode) * block_size)))
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		abort();
	}

	block->offset = 0;
	block->next = NULL;

	return block;
}

static struct _NodePtrBlock *
_node_allocator_create_ptr_block(int block_size)
{
	struct _NodePtrBlock *block;

	if(!(block = (struct _NodePtrBlock *)malloc(sizeof(struct _NodePtrBlock))))
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		abort();
	}

	if(!(block->nodes = (RBNode **)malloc(sizeof(RBNode *) * block_size)))
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		abort();
	}

	block->offset = 0;
	block->next = NULL;

	return block;
}

static void *
_node_allocator_alloc(Allocator *allocator)
{
	NodeAllocator *alloc = (NodeAllocator *)allocator;
	struct _NodeBlock *block;
	struct _NodePtrBlock *pblock;
	RBNode *node = NULL;

	/* try to get detached node */
	if(alloc->free_block)
	{
		node = alloc->free_block->nodes[alloc->free_block->offset--];

		if(alloc->free_block->offset == -1)
		{
			pblock = alloc->free_block;
			alloc->free_block = pblock->next;
			free(pblock->nodes);
			free(pblock);

			if(alloc->free_block)
			{
				alloc->free_block->offset--;
			}
		}

		return node;
	}

	/* test if we have reached end of the current block */
	if(alloc->block->offset < alloc->block_size)
	{
		/* end not reached => return current node & increment offset */
		node = &alloc->block->nodes[alloc->block->offset++];
	}
	else
	{
		/* end reached => create a new block & prepend it to our list */
		block = _node_allocator_create_block(alloc->block_size);
		block->next = alloc->block;
		alloc->block = block;

		/* return first node from current block & increment offset */
		node = &alloc->block->nodes[alloc->block->offset++];
	}

	return node;
}

static void
_node_allocator_free(Allocator *allocator, void *ptr)
{
	NodeAllocator *alloc = (NodeAllocator *)allocator;
	struct _NodePtrBlock *cur;

	if(!alloc->reuse_nodes)
	{
		return;
	}

	if(!(cur = alloc->free_block))
	{
		alloc->free_block = cur = _node_allocator_create_ptr_block(alloc->block_size);
	}
	else if(cur->offset == alloc->block_size)
	{
		cur = _node_allocator_create_ptr_block(alloc->block_size);
		cur->next = alloc->free_block;
		alloc->free_block = cur;
	}

	cur->nodes[cur->offset++] = (RBNode *)ptr;
}

static NodeAllocator *
_node_allocator_new(int block_size)
{
	NodeAllocator *alloc = NULL;

	if(!(alloc = (NodeAllocator *)malloc(sizeof(NodeAllocator))))
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		abort();
	}

	((Allocator *)alloc)->alloc = _node_allocator_alloc;
	((Allocator *)alloc)->free = _node_allocator_free;

	alloc->block = _node_allocator_create_block(block_size);
	alloc->reuse_nodes = true;
	alloc->free_block = NULL;
	alloc->block_size = block_size;

	return alloc;
}

static void
_node_allocator_destroy(NodeAllocator *alloc)
{
	struct _NodeBlock *block;
	struct _NodeBlock *iter;
	struct _NodePtrBlock *pblock;
	struct _NodePtrBlock *piter;

	/* free memory blocks & list */
	iter = alloc->block;

	while(iter)
	{
		block = iter;
		iter = iter->next;
		free(block->nodes);
		free(block);
	}

	/* free list containing free nodes */
	piter = alloc->free_block;

	while(piter)
	{
		pblock = piter;
		piter = piter->next;
		free(pblock);
	}

	free(alloc);
}

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
hashtable_new(int32_t size, HashFunc hash_func, CompareFunc compare_keys, FreeFunc free_key, FreeFunc free_value)
{
	HashTable *table;

	assert(size > 0);
	assert(size < INT32_MAX - 1);
	assert(hash_func != NULL);
	assert(compare_keys != NULL);

	if(!(table = (HashTable *)malloc(sizeof(HashTable))))
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		abort();
	}

	if(!(table->buckets = (RBTree **)calloc(size, sizeof(RBTree *))))
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		abort();
	}

	if(!(table->pool = (RBTree *)malloc(size * sizeof(RBTree))))
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		abort();
	}

	if(size >= 512)
	{
		if(!(table->allocator = (Allocator *)_node_allocator_new(512)))
		{
			fprintf(stderr, "Couldn't allocate memory.\n");
			abort();
		}
	}
	else
	{
		table->allocator = NULL;
	}

	table->compare_keys = compare_keys;
	table->free_key = free_key;
	table->free_value = free_value;
	table->size = size;
	table->hash = hash_func;
	table->poolptr = table->pool;
	table->count = 0;

	return table;
}

static inline void
_hashtable_destroy_bucket(HashTable *table, int index)
{
	if(table->buckets[index])
	{
		/* check if we're using an allocator and if we don't have to free keys & values */
		if(table->allocator && !table->free_key && !table->free_value)
		{
			/*
			 * memory allocated for nodes will be freed when destroying the allocator, so
			 * we can set the root node of the tree to NULL to improve performance
			 */
			table->buckets[index]->root = NULL;
		}

		rbtree_free(table->buckets[index]);
	}
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
		_hashtable_destroy_bucket(table, i);
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
	int i;
	int to;

	#if defined(PTHREADS)
	pthread_t thread;
	pthread_attr_t attr;
	#elif defined(WIN32)
	HANDLE thread = NULL;
	#endif

	if(table->allocator)
	{
		/* we don't have to attach freed nodes to the list of available nodes anymore */
		((NodeAllocator *)table->allocator)->reuse_nodes = false;
	}

	#if defined(PTHREADS) || defined(WIN32)
	if(table->size >= 512)
	{
		to = table->size / 2;

		#ifdef PTHREADS
		pthread_attr_init(&attr);
		pthread_create(&thread, NULL, _hashtable_destroy_worker, table);
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
		_hashtable_destroy_bucket(table, i);
	}

	#if defined(PTHREADS)
	if(table->size >= 512)
	{
		pthread_join(thread, NULL);
		pthread_detach(thread);
	}
	#elif defined(WIN32)
	if(thread)
	{
		WaitForSingleObject(thread, INFINITE);
	}
	#endif

	if(table->allocator)
	{
		_node_allocator_destroy((NodeAllocator *)table->allocator);
	}

	free(table->pool);
	free(table);
}

void
hashtable_set(HashTable *table, void * restrict key, void * restrict value, bool overwrite_key)
{
	uint32_t index;
	RBTree *bucket;

	assert(table != NULL);
	assert(key != NULL);

	index = HASHTABLE_INDEX(table, key);
	assert(index < table->size);

	if(!(bucket = table->buckets[index]))
	{
		bucket = table->buckets[index] = table->poolptr++;
		rbtree_init(bucket, table->compare_keys, table->free_key, table->free_value, table->allocator);
	}

	if(rbtree_set(bucket, key, value, overwrite_key) == RBTREE_INSERT_RESULT_NEW)
	{
		table->count++;
	}
}

void
hashtable_remove(HashTable *table, const void *key)
{
	RBTree *bucket;

	assert(table != NULL);
	assert(key != NULL);

	if((bucket = table->buckets[HASHTABLE_INDEX(table, key)]))
	{
		if(rbtree_remove(bucket, key))
		{
			table->count--;
		}
	}
}

void *
hashtable_lookup(HashTable *table, const void *key)
{
	RBTree *bucket;

	assert(table != NULL);
	assert(key != NULL);

	if((bucket = table->buckets[HASHTABLE_INDEX(table, key)]))
	{
		return rbtree_lookup(bucket, key);
	}

	return NULL;
}

bool
hashtable_key_exists(HashTable *table, const void *key)
{
	RBTree *bucket;

	if((bucket = table->buckets[HASHTABLE_INDEX(table, key)]))
	{
		return rbtree_key_exists(bucket, key);
	}

	return false;
}

void
hashtable_iter_init(HashTable *table, HashTableIter *iter)
{
	memset(iter, 0, sizeof(HashTableIter));
	iter->table = table;
}

void
hashtable_iter_free(HashTableIter *iter)
{
	assert(iter != NULL);

	if(iter->rbiter_init)
	{
		rbtree_iter_free(&iter->rbiter);
	}
}

static inline bool
_hashtable_iter_get_next_bucket(HashTableIter *iter)
{
	while(!iter->rbiter_set && iter->offset < iter->table->size)
	{
		if(iter->table->buckets[iter->offset])
		{
			if(iter->rbiter_init)
			{
				rbtree_iter_reuse(iter->table->buckets[iter->offset], &iter->rbiter);
			}
			else
			{
				rbtree_iter_init(iter->table->buckets[iter->offset], &iter->rbiter);
				iter->rbiter_init = true;
			}

			iter->rbiter_set = true;
		}

		iter->offset++;
	}

	return iter->rbiter_set;
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
		if(iter->rbiter_set)
		{
			if((iter->rbiter_set = rbtree_iter_next(&iter->rbiter)))
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
		}
	}
}

void inline *
hashtable_iter_get_key(HashTableIter *iter)
{
	assert(iter != NULL);

	if(iter->rbiter_set)
	{
		return rbtree_iter_get_key(&iter->rbiter);
	}

	return false;
}

void inline *
hashtable_iter_get_value(HashTableIter *iter)
{
	assert(iter != NULL);

	if(iter->rbiter_set)
	{
		return rbtree_iter_get_value(&iter->rbiter);
	}

	return false;
}

