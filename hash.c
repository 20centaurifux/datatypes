/***************************************************************************
    begin........: May 2012
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
 * \file hash.c
 * \brief Hash functions.
 * \author Sebastian Fedrau <sebastian.fedrau@gmail.com>
 */
#include <stdio.h>
#include <assert.h>

#include "hash.h"

uint32_t
str_hash(const void *ptr)
{
	assert(ptr != NULL);

	const char *plain = ptr;
	uint32_t hash = 0;

	while(*plain)
	{
		hash = *plain++ + (hash << 6) + (hash << 16) - hash;
	}

	return hash;
}

uint32_t
direct_hash(const void *ptr)
{
	uintptr_t v = (uintptr_t)ptr;

	if(v > UINT32_MAX)
	{
		fprintf(stderr, "%s(): integer overflow.\n", __func__);
		v = UINT32_MAX;
	}

	return (uint32_t)v;
}

