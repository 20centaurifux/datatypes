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
/*!
 * \file datatypes.c
 * \brief General declarations.
 * \author Sebastian Fedrau <sebastian.fedrau@gmail.com>
 * \version 0.1.0
 * \date 27. June 2012
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

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

uint32_t inline
str_hash(const char *plain)
{
	uint32_t hash = 0;

	while(*plain)
	{
		hash = *plain++ + (hash << 6) + (hash << 16) - hash;
	}

	return hash;
}

int32_t
direct_compare(const void *a, const void *b)
{
	size_t result = (size_t)a - (size_t)b;

	assert(result > INT32_MIN);
	assert(result < INT32_MAX);

	return (int32_t)result;
}

inline bool
direct_equal(const void *a, const void *b)
{
	return a == b;
}

