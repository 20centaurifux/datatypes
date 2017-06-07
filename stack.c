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
 * \file stack.c
 * \brief A generic stack.
 * \author Sebastian Fedrau <sebastian.fedrau@gmail.com>
 */
#include <stdlib.h>
#include <assert.h>

#include "stack.h"

bool
stack_pop(Stack *stack, void **data)
{
	assert(stack != NULL);

	if(slist_count(stack))
	{
		void *ptr = slist_pop(stack);

		if(data)
		{
			*data = ptr;
		}

		return true;
	}

	if(data)
	{
		*data = NULL;
	}

	return false;
}

bool
stack_head(Stack *stack, void **data)
{
	SListItem *iter;

	assert(stack != NULL);
	assert(data != NULL);

	if((iter = slist_head(stack)))
	{
		*data = iter->data;

		return true;
	}

	return false;
}

