#ifndef __DATATYPES_H__
#define __DATATYPES_H__

#include <stdbool.h>
#include <stdint.h>

typedef int32_t (*CompareFunc)(const void *a, const void *b);
typedef bool (*EqualFunc)(const void *a, const void *b);
typedef void (*FreeFunc)(void *p);

#define str_compare (CompareFunc)strcmp
int32_t int_compare(const void *a, const void *b);

bool str_equal(const void * restrict a, const void * restrict b);
bool int_equal(const void * restrict a, const void * restrict b);

#endif

