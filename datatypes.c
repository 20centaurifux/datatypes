#include <stdlib.h>
#include <stdio.h>

#include "datatypes.h"

bool
str_equal(const void * restrict a, const void * restrict b)
{
	const char *s0 = (const char *)a;
	const char *s1 = (const char *)b;

	while(*s0 && *s1)
	{
		if(*s0++ != *s1++)
		{
			return false;
		}
	}

	return true;
}

int32_t
int_compare(const void *a, const void *b)
{
	return (int)a - (int)b;
}

inline bool
int_equal(const void * restrict a, const void * restrict b)
{
	return (int *)a == (int *)b;
}

