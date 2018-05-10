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
 * \file pool.h
 * \brief Allocate memory blocks of same sizes.
 * \author Sebastian Fedrau <sebastian.fedrau@gmail.com>
 */
#ifndef POOL_H
#define POOL_H

#include <stdint.h>

/**
 *\struct Pool
 *\brief A memory pool.
 */
typedef struct _Pool
{
	/*! Function to allocate memory. */
	void *(*alloc)(struct _Pool *pool);
	/*! Function to free memory. */
	void (*free)(struct _Pool *pool, void *ptr);
} Pool;

/**
 *\struct MemoryPool
 *\brief This memory pool allocates memorys of memory and grows automatically.
 */
typedef struct
{
	/*! Padding.*/
	Pool padding;

	/**
	 *\struct _MemoryBlock
	 *\brief Memorys of memory are stored in a singly-linked list.
	 *
	 *\var block
	 *\brief First memory block.
	 */
	struct _MemoryBlock
	{
		/*! The allocated memory block. */
		int8_t *items;
		/*! Offset of the next available address. */
		size_t offset;
		/*! Pointer to next available memory block or NULL. */
		struct _MemoryBlock *next;
	} *block;
	/**
	 *\struct _MemoryPtrBlock
	 *\brief Freed pointers are stored in memorys holding addresses.
	 *
	 *\var free_block
	 *\brief First pointer block.
	 */
	struct _MemoryPtrBlock
	{
		/*! Array containing freed pointers. */
		void **items;
		/*! Offset to find next available position in items array. */
		size_t offset;
		/*! Pointer to next memory or NULL. */
		struct _MemoryPtrBlock *next;
	} *free_block;
	/*! Number of items a memory memory can hold. */
	size_t block_size;
	/*! Size of an allocated item. */
	size_t item_size;
} MemoryPool;

/**
 *\param item_size size of allocated items
 *\param block_size number of elements a memory can hold
 *\return a new pool
 *
 * Creates a new MemoryPool.
 */
MemoryPool *memory_pool_new(size_t item_size, size_t block_size);

/**
 *\param pool a memory pool
 *
 * Destroys the given MemoryPool.
 */
void memory_pool_destroy(MemoryPool *pool);

#endif

