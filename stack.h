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
 * \file stack.h
 * \brief Generic stack.
 * \author Sebastian Fedrau <sebastian.fedrau@gmail.com>
 */
#ifndef STACK_H
#define STACK_H

#include "slist.h"

/**
 *\typedef Stack
 *\brief Generic stack.
 */
typedef SList Stack;

/**
 *\typedef StackItem
 *\brief Holds stack item data & pointer to next element.
 */
typedef SList StackItem;

/**
 *\param compare function to compare item data
 *\param free function to free item data or NULL
 *\param pool a user-defined memory pool for creating/destroying StackItems or NULL
 *\return a new Stack
 *
 * Creates a new Stack.
 */
#define stack_new(compare, free, pool) slist_new(compare, free, pool)

/**
 *\param stack a Stack
 *\param compare function to compare item data
 *\param free function to free item data or NULL
 *\param pool a user-defined memory pool for creating/destroying StackItems or NULL
 *
 * Initializes a Stack.
 */
#define stack_init(stack, compare, free, pool) slist_init(stack, compare, free, pool)

/**
 *\param stack a Stack
 *
 * Frees all items in the stack and the stack pointer.
 */
#define stack_destroy(stack) slist_destroy(stack)

/**
 *\param stack a Stack
 *
 * Frees all items in the stack without freeing the stack pointer.
 */
#define stack_free(stack) slist_free(stack)

/**
 *\param stack a Stack
 *\param data data to push
 *
 * Pushs data onto the stack.
 */
#define stack_push(stack, data) slist_prepend(stack, data)

/**
 *\param stack a Stack
 *\param data location to store data
 *\return true if stack is not empty
 *
 * Gets first element from stack without removing it.
 */
bool stack_head(const Stack *stack, void **data);

/**
 *\param stack a Stack
 *\param data location to store data
 *\return true if stack is not empty
 *
 * Pops the first element from the stack.
 */
bool stack_pop(Stack *stack, void **data);

/**
 *\param stack a Stack
 *
 * Clears a stack.
 */
#define stack_clear(stack) slist_clear(stack)

/**
 *\param stack a Stack
 *\return number of items
 *
 * Gets the number of stored items.
 */
#define stack_count(stack) slist_count(stack)

#endif

