/***************************************************************************
    begin........: June 2012
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
 * \file pool.c
 * \brief Allocate memory blocks of same sizes.
 * \author Sebastian Fedrau <sebastian.fedrau@gmail.com>
 */
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "pool.h"

static struct _MemoryBlock *
_memory_pool_create_block(const MemoryPool *pool)
{
	struct _MemoryBlock *block;

	if(!(block = (struct _MemoryBlock *)malloc(sizeof(struct _MemoryBlock))))
	{
		perror("malloc()");
		abort();
	}

	if(!(block->items = malloc(pool->item_size * pool->block_size)))
	{
		perror("malloc()");
		abort();
	}

	block->offset = 0;
	block->next = NULL;

	return block;
}

static struct _MemoryPtrBlock *
_memory_pool_create_ptr_block(const MemoryPool *pool)
{
	struct _MemoryPtrBlock *block;

	if(!(block = (struct _MemoryPtrBlock *)malloc(sizeof(struct _MemoryPtrBlock))))
	{
		perror("malloc()");
		abort();
	}

	if(!(block->items = (void **)malloc(pool->block_size * sizeof(void *))))
	{
		perror("malloc()");
		abort();
	}

	block->offset = 0;
	block->next = NULL;

	return block;
}

static void *
_memory_pool_alloc(Pool *alloc)
{
	MemoryPool *pool = (MemoryPool *)alloc;
	struct _MemoryBlock *block;
	struct _MemoryPtrBlock *pblock;
	void *item = NULL;

	/* try to get detached item */
	if(pool->free_block)
	{
		assert(pool->free_block->offset > 0);

		item = pool->free_block->items[--pool->free_block->offset];

		if(!pool->free_block->offset)
		{
			pblock = pool->free_block;
			pool->free_block = pblock->next;
			free(pblock->items);
			free(pblock);
		}

		return item;
	}

	/* test if we have reached end of the current block */
	if(pool->block->offset < pool->block_size)
	{
		/* end not reached => return current item & increment offset */
		item = pool->block->items + (pool->item_size * pool->block->offset++);
	}
	else
	{
		/* end reached => create a new block & prepend it to our list */
		block = _memory_pool_create_block(pool);
		block->next = pool->block;
		pool->block = block;

		/* return first item from current block & increment offset */
		item = pool->block->items;
		++pool->block->offset;
	}

	return item;
}

static void
_memory_pool_free(Pool *alloc, void *item)
{
	MemoryPool *pool = (MemoryPool *)alloc;
	struct _MemoryPtrBlock *cur;

	if(!(cur = pool->free_block))
	{
		pool->free_block = cur = _memory_pool_create_ptr_block(pool);
	}
	else if(cur->offset == pool->block_size)
	{
		cur = _memory_pool_create_ptr_block(pool);
		cur->next = pool->free_block;
		pool->free_block = cur;
	}

	cur->items[cur->offset++] = item;
}

MemoryPool *
memory_pool_new(size_t item_size, size_t block_size)
{
	MemoryPool *pool;

	assert(item_size > 1);
	assert(block_size > 1);
	assert(block_size < SIZE_MAX / item_size);
	assert(block_size < SIZE_MAX / sizeof(void *));

	if(!(pool = (MemoryPool *)malloc(sizeof(MemoryPool))))
	{
		perror("malloc()");
		abort();
	}

	pool->free_block = NULL;
	pool->item_size = item_size;
	pool->block_size = block_size;
	pool->block = _memory_pool_create_block(pool);

	((Pool *)pool)->alloc = _memory_pool_alloc;
	((Pool *)pool)->free = _memory_pool_free;

	return pool;
}

void
memory_pool_destroy(MemoryPool *pool)
{
	struct _MemoryBlock *iter;
	struct _MemoryPtrBlock *piter;

	assert(pool != NULL);

	/* free memory blocks & list */
	iter = pool->block;

	while(iter)
	{
		struct _MemoryBlock *block;

		block = iter;
		iter = iter->next;
		free(block->items);
		free(block);
	}

	/* free list containing free items */
	piter = pool->free_block;

	while(piter)
	{
		struct _MemoryPtrBlock *pblock;

		pblock = piter;
		piter = piter->next;
		free(pblock->items);
		free(pblock);
	}

	free(pool);
}

