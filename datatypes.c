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
 * \file datatypes.c
 * \brief General declarations.
 * \author Sebastian Fedrau <sebastian.fedrau@gmail.com>
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>

#include "datatypes.h"

bool
str_equal(const void * restrict a, const void * restrict b)
{
	const char *s0 = (const char *)a;
	const char *s1 = (const char *)b;

	assert(a != NULL);
	assert(b != NULL);

	while(*s0 && *s1)
	{
		if(*s0++ != *s1++)
		{
			return false;
		}
	}

	return *s0 == *s1;
}

inline uint32_t
str_hash(const void *ptr)
{
	const char *plain = ptr;
	uint32_t hash = 0;

	assert(ptr != NULL);

	while(*plain)
	{
		hash = *plain++ + (hash << 6) + (hash << 16) - hash;
	}

	return hash;
}

int32_t
direct_compare(const void *a, const void *b)
{
	ptrdiff_t result = a - b;

	assert(a != NULL);
	assert(b != NULL);

	if(result > INT32_MAX)
	{
		fprintf(stderr, "%s: warning, integer overflow.\n", __func__);
		result = INT32_MAX;
	}
	else if(result < INT32_MIN)
	{
		fprintf(stderr, "%s: warning, integer underflow.\n", __func__);
		result = INT32_MIN;
	}

	return (int32_t)result;
}

inline bool
direct_equal(const void *a, const void *b)
{
	assert(a != NULL);
	assert(b != NULL);

	return a == b;
}

