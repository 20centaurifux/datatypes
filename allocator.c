/***************************************************************************
    begin........: June 2012
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
 * \file allocator.c
 * \brief Allocate chunks of memory.
 * \author Sebastian Fedrau <lord-kefir@arcor.de>
 * \version 0.1.0
 * \date 27. June 2012
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "allocator.h"

static struct _MemoryBlock *
_g_allocator_create_block(GAllocator *allocator)
{
	struct _MemoryBlock *block;

	if(!(block = (struct _MemoryBlock *)malloc(sizeof(struct _MemoryBlock))))
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		abort();
	}

	if(!(block->items = malloc(allocator->item_size * allocator->block_size)))
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		abort();
	}

	block->offset = 0;
	block->next = NULL;

	return block;
}

static struct _MemoryPtrBlock *
_g_allocator_create_ptr_block(GAllocator *allocator)
{
	struct _MemoryPtrBlock *block;

	if(!(block = (struct _MemoryPtrBlock *)malloc(sizeof(struct _MemoryPtrBlock))))
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		abort();
	}

	if(!(block->items = (void **)malloc(allocator->block_size * sizeof(void *))))
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		abort();
	}

	block->offset = 0;
	block->next = NULL;

	return block;
}

static void *
_g_allocator_alloc(Allocator *alloc)
{
	GAllocator *allocator = (GAllocator *)alloc;
	struct _MemoryBlock *block;
	struct _MemoryPtrBlock *pblock;
	void *item = NULL;

	/* try to get detached item */
	if(allocator->free_block)
	{
		item = allocator->free_block->items[--allocator->free_block->offset];

		if(!allocator->free_block->offset)
		{
			pblock = allocator->free_block;
			allocator->free_block = pblock->next;
			free(pblock->items);
			free(pblock);

			if(allocator->free_block)
			{
				allocator->free_block->offset--;
			}
		}

		return item;
	}

	/* test if we have reached end of the current block */
	if(allocator->block->offset < allocator->block_size)
	{
		/* end not reached => return current item & increment offset */
		item = allocator->block->items + (allocator->item_size * allocator->block->offset++);
	}
	else
	{
		/* end reached => create a new block & prepend it to our list */
		block = _g_allocator_create_block(allocator);
		block->next = allocator->block;
		allocator->block = block;

		/* return first item from current block & increment offset */
		item = allocator->block->items;
		allocator->block->offset++;
	}

	return item;
}

static void
_g_allocator_free(Allocator *alloc, void *item)
{
	GAllocator *allocator = (GAllocator *)alloc;
	struct _MemoryPtrBlock *cur;

	if(!(cur = allocator->free_block))
	{
		allocator->free_block = cur = _g_allocator_create_ptr_block(allocator);
	}
	else if(cur->offset == allocator->block_size)
	{
		cur = _g_allocator_create_ptr_block(allocator);
		cur->next = allocator->free_block;
		allocator->free_block = cur;
	}

	cur->items[cur->offset++] = item;
}

GAllocator *
g_allocator_new(int item_size, int block_size)
{
	GAllocator *allocator;

	assert(item_size > 1);
	assert(block_size > 1);

	if(!(allocator = (GAllocator *)malloc(sizeof(GAllocator))))
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		abort();
	}

	allocator->free_block = NULL;
	allocator->item_size = item_size;
	allocator->block_size = block_size;
	allocator->block = _g_allocator_create_block(allocator);

	((Allocator *)allocator)->alloc = _g_allocator_alloc;
	((Allocator *)allocator)->free = _g_allocator_free;

	return allocator;
}

void
g_allocator_destroy(GAllocator *allocator)
{
	struct _MemoryBlock *block;
	struct _MemoryBlock *iter;
	struct _MemoryPtrBlock *pblock;
	struct _MemoryPtrBlock *piter;

	/* free memory blocks & list */
	iter = allocator->block;

	while(iter)
	{
		block = iter;
		iter = iter->next;
		free(block->items);
		free(block);
	}

	/* free list containing free items */
	piter = allocator->free_block;

	while(piter)
	{
		pblock = piter;
		piter = piter->next;
		free(pblock->items);
		free(pblock);
	}

	free(allocator);
}


