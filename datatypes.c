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
 * \file datatypes.c
 * \brief General declarations.
 * \author Sebastian Fedrau <lord-kefir@arcor.de>
 * \version 0.1.0
 * \date 27. June 2012
 */

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

inline bool
direct_equal(const void * restrict a, const void * restrict b)
{
	return a == b;
}

