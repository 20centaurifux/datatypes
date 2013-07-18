/***************************************************************************
    begin........: June 2012
    copyright....: Sebastian Fedrau
    email........: lord-kefir@arcor.de
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
/*!
 * \file allocator.h
 * \brief Allocate chunks of memory.
 * \author Sebastian Fedrau <lord-kefir@arcor.de>
 * \version 0.1.0
 * \date 27. June 2012
 */

#ifndef __ALLOCATOR_H__
#define __ALLOCATOR_H__

#include <stdint.h>

/**
 *\struct Allocator
 *\brief A memory allocator.
 */
typedef struct _Allocator
{
	/*! Function to allocate memory. */
	void *(*alloc)(struct _Allocator *allocator);
	/*! Function to free memory. */
	void (*free)(struct _Allocator *allocator, void *ptr);
} Allocator;

/**
 *\struct ChunkAllocator
 *\brief This memory allocator allocates chunks of memory and grows automatically.
 *
 */
typedef struct
{
	/*! Padding.*/
	Allocator padding;

	/**
	 *\struct _MemoryBlock
	 *\brief Chunks of memory are stored in a singly-linked list.
	 *
	 *\var block
	 *\brief First memory block.
	 */
	struct _MemoryBlock
	{
		/*! The allocated memory block. */
		int8_t *items;
		/*! Offset of the next available address. */
		int offset;
		/*! Pointer to next available memory block or NULL. */
		struct _MemoryBlock *next;
	} *block;
	/**
	 *\struct _MemoryPtrBlock
	 *\brief Freed pointers are stored in chunks holding addresses.
	 *
	 *\var free_block
	 *\brief First pointer block.
	 */
	struct _MemoryPtrBlock
	{
		/*! Array containing freed pointers. */
		void **items;
		/*! Offset to detect next index to store a freed pointer. */
		int offset;
		/*! Pointer to next chunk or NULL. */
		struct _MemoryPtrBlock *next;
		/*! Pointer to previous chunk or NULL. */
		struct _MemoryPtrBlock *prev;
	} *free_block;
	/*! Number of items a memory chunk can hold. */
	int block_size;
	/*! Size of an allocated item. */
	int item_size;
} ChunkAllocator;

/**
 *\param item_size size of allocated items
 *\param block_size number of elements a chunk can hold
 *\return a new allocator
 *
 * Creates a new memory allocator.
 */
ChunkAllocator *chunk_allocator_new(int item_size, int block_size);

/**
 *\param allocator a memory allocator
 *
 * Destroys the given memory allocator.
 */
void chunk_allocator_destroy(ChunkAllocator *allocator);

#endif

