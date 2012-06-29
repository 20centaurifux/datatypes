/***************************************************************************
    begin........: June 2012
    copyright....: Sebastian Fedrau
    email........: lord-kefir@arcor.de
 ***************************************************************************/

/***************************************************************************
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.
 ***************************************************************************/
/*!
 * \file queue.h
 * \brief A generic queue.
 * \author Sebastian Fedrau <lord-kefir@arcor.de>
 * \version 0.1.0
 * \date 29. June 2012
 */

#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "stack.h"

/**
 *\struct Queue
 *\brief A generic queue.
 */
typedef SList Queue;

/**
 *\struct QueueItem
 *\brief Holds queue item data & pointer to next element.
 */
typedef SList QueueItem;

/**
 *\param equals function to compare item data
 *\param free function to free item data or NULL
 *\param allocator a user-defined memory allocator for creating/destroying QueueItems or NULL
 *\return a new Queue
 *
 * Creates a new Queue.
 */
#define queue_new(equals, free, allocator) stack_new(equals, free, allocator)

/**
 *\param queue a Queue
 *\param equals function to compare item data
 *\param free function to free item data or NULL
 *\param allocator a user-defined memory allocator for creating/destroying QueueItems or NULL
 *
 * Initializes a Queue.
 */
#define queue_init(queue, equals, free, allocator) stack_init(queue, equals, free, allocator)

/**
 *\param queue a Queue
 *
 * Destroys all items in the queue. Frees also memory allocated for the Queue instance.
 */
#define queue_destroy(queue) stack_destroy(queue)

/**
 *\param queue a Queue
 *
 * Destroys all items in the queue.
 */
#define queue_free(queue) stack_free(queue)

/**
 *\param queue a Queue
 *\param data data to push
 *
 * Pushs data onto the queue.
 */
#define queue_push(queue, data) slist_append(queue, data)

/**
 *\param queue a Queue
 *\param data location to store data
 *\return true if queue is not empty
 *
 * Removes first element from queue.
 */
#define queue_pop(queue, data) stack_pop(queue, data)

/**
 *\param queue a Queue
 *
 * Clears a queue.
 */
#define queue_clear(queue) stack_clear(queue)

/**
 *\param queue a Queue
 *\return number of items
 *
 * Gets the number of items.
 */
#define queue_count(queue) stack_count(queue)

#endif


