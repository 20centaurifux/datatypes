#ifndef __ALLOCATOR_H__
#define __ALLOCATOR_H__

typedef struct _Allocator
{
	void *(*alloc)(struct _Allocator *allocator);
	void (*free)(struct _Allocator *allocator, void *ptr);
} Allocator;

typedef struct
{
	Allocator padding;

	/* store blocks of memory in a list */
	struct _MemoryBlock
	{
		int8_t *items;
		int offset;
		struct _MemoryBlock *next;
	} *block;
	/* store free nodes in blocks containing pointers */
	struct _MemoryPtrBlock
	{
		void **items;
		int offset;
		struct _MemoryPtrBlock *next;
		struct _MemoryPtrBlock *prev;
	} *free_block;
	int block_size;
	int item_size;
} HungryAllocator;

HungryAllocator *hungry_allocator_new(int item_size, int block_size);
void hungry_allocator_destroy(HungryAllocator *allocator);

#endif
