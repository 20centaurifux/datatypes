#ifndef __DATATYPES_H__
#define __DATATYPES_H__

#include <stdbool.h>
#include <stdint.h>

typedef int32_t (*CompareFunc)(const void *a, const void *b);
typedef bool (*EqualFunc)(const void *a, const void *b);
typedef void (*FreeFunc)(void *p);
#define str_compare (CompareFunc)strcmp

typedef struct _Allocator
{
	void *(*alloc)(struct _Allocator *allocator);
	void (*free)(struct _Allocator *allocator, void *ptr);
} Allocator;

#endif
