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
 * \brief A generic stack.
 * \author Sebastian Fedrau <sebastian.fedrau@gmail.com>
 */
#ifndef STACK_H
#define STACK_H

#include "slist.h"

/**
 *\typedef Stack
 *\brief A generic stack.
 */
typedef SList Stack;

/**
 *\typedef StackItem
 *\brief Holds stack item data & pointer to next element.
 */
typedef SList StackItem;

/**
 *\param equals function to compare item data
 *\param free function to free item data or NULL
 *\param allocator a user-defined memory allocator for creating/destroying StackItems or NULL
 *\return a new Stack
 *
 * Creates a new Stack.
 */
#define stack_new(equals, free, allocator) slist_new(equals, free, allocator)

/**
 *\param stack a Stack
 *\param equals function to compare item data
 *\param free function to free item data or NULL
 *\param allocator a user-defined memory allocator for creating/destroying StackItems or NULL
 *
 * Initializes a Stack.
 */
#define stack_init(stack, equals, free, allocator) slist_init(stack, equals, free, allocator)

/**
 *\param stack a Stack
 *
 * Destroys all items in the stack. Frees also memory allocated for the Stack instance.
 */
#define stack_destroy(stack) slist_destroy(stack)

/**
 *\param stack a Stack
 *
 * Destroys all items in the stack.
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
 * Removes first element from stack.
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

