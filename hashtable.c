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
		exit(EXIT_FAILURE);
	}

	if(!(block->nodes = (RBNode *)malloc(sizeof(RBNode) * block_size)))
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		exit(EXIT_FAILURE);
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
		exit(EXIT_FAILURE);
	}

	if(!(block->nodes = (RBNode **)malloc(sizeof(RBNode *) * block_size)))
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		exit(EXIT_FAILURE);
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
		exit(EXIT_FAILURE);
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
	char c;

	while((c = *plain++))
	{
		hash = c + (hash << 6) + (hash << 16) - hash;
	}

	return hash;
}
HashTable *
hashtable_new(uint32_t size, HashFunc hash_func, CompareFunc compare_keys, FreeFunc free_key, FreeFunc free_value)
{
	HashTable *table;

	assert(size > 0);
	assert(hash_func != NULL);
	assert(compare_keys != NULL);

	if(!(table = (HashTable *)malloc(sizeof(HashTable))))
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		exit(EXIT_FAILURE);
	}

	if(!(table->buckets = (RBTree **)calloc(size, sizeof(RBTree *))))
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		exit(EXIT_FAILURE);
	}

	if(!(table->pool = (RBTree *)malloc(size * sizeof(RBTree))))
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		exit(EXIT_FAILURE);
	}

	if(size > 1024)
	{
		if(!(table->allocator = (Allocator *)_node_allocator_new(size / 512)))
		{
			fprintf(stderr, "Couldn't allocate memory.\n");
			exit(EXIT_FAILURE);
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

#define _hashtable_destroy_bucket(table, i) if(table->buckets[i]) rbtree_free(table->buckets[i])

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
		((NodeAllocator *)table->allocator)->reuse_nodes = false;
	}

	#if defined(PTHREADS) || defined(WIN32)
	if(table->size > 512)
	{
		to = table->size / 2;

		#ifdef PTHREADS
		pthread_create(&thread, NULL, _hashtable_destroy_worker, table);
		pthread_attr_init(&attr);
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
	if(table->size > 512)
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

bool
hashtable_foreach(HashTable *table, ForeachKeyValuePairFunc foreach, void *user_data)
{
	uint32_t i = 0;

	assert(table != NULL);
	assert(foreach != NULL);

	for(i = 0; i < table->size; ++i)
	{
		if(table->buckets[i])
		{
			if(!rbtree_foreach(table->buckets[i], foreach, user_data))
			{
				return false;
			}
		}
	}

	return true;
}

