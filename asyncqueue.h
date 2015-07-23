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
 * \file asyncqueue.h
 * \brief An asynchronous queue.
 * \author Sebastian Fedrau <lord-kefir@arcor.de>
 * \version 0.1.0
 * \date 23. July 2015
 */

#ifndef __ASYNCQUEUE_H__
#define __ASYNCQUEUE_H__

#include <pthread.h>
#include "queue.h"

/**
 *\struct AsyncQueue
 *\brief Asynchronous queue implementation.
 */
typedef struct
{
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	Queue queue;
	uint32_t waiting;
} AsyncQueue;

/**
 *\param equals function to compare item data
 *\param free function to free item data or NULL
 *\param allocator a user-defined memory allocator for creating/destroying QueueItems or NULL
 *\return a new Queue
 *
 * Creates a new AsyncQueue.
 */
AsyncQueue *async_queue_new(EqualFunc equals, FreeFunc free, Allocator *allocator);

/**
 *\param queue an AsyncQueue
 *\param equals function to compare item data
 *\param free function to free item data or NULL
 *\param allocator a user-defined memory allocator for creating/destroying QueueItems or NULL
 *
 * Initializes an AsyncQueue.
 */
void async_queue_init(AsyncQueue *queue, EqualFunc equals, FreeFunc free, Allocator *allocator);

/**
 *\param queue an AsyncQueue
 *
 * Destroys all items in the queue. Frees also memory allocated for the AsyncQueue instance.
 */
void async_queue_destroy(AsyncQueue *queue);

/**
 *\param queue an AsyncQueue
 *
 * Destroys all items in the queue.
 */
void async_queue_free(AsyncQueue *queue);

/**
 *\param queue an AsyncQueue
 *\param data data to push
 *
 * Pushs data onto the queue.
 */
void async_queue_push(AsyncQueue *queue, void *data);

/**
 *\param queue an AsyncQueue
 *\param data location to store data of removed element
 *\return true on success
 *
 * Pops data from the queue.
 */
bool async_queue_pop(AsyncQueue *queue, void *data);

/**
 *\param queue an AsyncQueue
 *\param data location to store data of removed element
 *\param ms timeout in milliseconds
 *\return true on success
 *
 * Pops data from the queue. Blocks for ms milliseconds.
 */
bool async_queue_pop_timeout(AsyncQueue *queue, void *data, uint32_t ms);

/**
 *\param queue an AsyncQueue
 *
 * Clears a queue.
 */
void async_queue_clear(AsyncQueue *queue);

/**
 *\param queue a Queue
 *\return number of items
 *
 * Gets the number of stored items.
 */
size_t async_queue_count(AsyncQueue *queue);
#endif

