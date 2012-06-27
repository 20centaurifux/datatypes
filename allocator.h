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
 *\struct _Allocator
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
 *\struct GAllocator
 *\brief This memory allocator allocates blocks of memory and grows automatically.
 *
 */
typedef struct
{
	/*! Padding.*/
	Allocator padding;

	/**
	 *\struct _MemoryBlock
	 *\brief Blocks of memory are stored in a singly-linked list.
	 */
	struct _MemoryBlock
	{
		/*! The allocated memory block. */
		int8_t *items;
		/*! Offset to find next free address of the memory block. */
		int offset;
		/*! Pointer to next memory block. */
		struct _MemoryBlock *next;
	} *block;
	/**
	 *\struct _MemoryPtrBlock
	 *\brief Freed pointers are stored in blocks holding addresses.
	 */
	struct _MemoryPtrBlock
	{
		/*! Array containing freed pointers. */
		void **items;
		/*! Offset to detect next index to store a freed pointer. */
		int offset;
		/*! Pointer to next memory block */
		struct _MemoryPtrBlock *next;
		/*! Pointer to previous memory block */
		struct _MemoryPtrBlock *prev;
	} *free_block;
	/*! NUmber of memory blocks. */
	int block_size;
	/*! Size of an allocated item. */
	int item_size;
} GAllocator;

/**
 *\param item_size size of allocated items
 *\param block_size size of allocated memory blocks
 *\return a new allocator
 *
 * Creates a new memory allocator.
 */
GAllocator *g_allocator_new(int item_size, int block_size);

/**
 *\param allocator a memory allocator
 *
 * Destroys the given memory allocator.
 */
void g_allocator_destroy(GAllocator *allocator);

#endif

