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
 * \file asyncqueue.h
 * \brief An asynchronous queue.
 * \author Sebastian Fedrau <sebastian.fedrau@gmail.com>
 */
#ifdef WITH_PTHREAD

#ifndef ASYNCQUEUE_H
#define ASYNCQUEUE_H

#include <pthread.h>
#include "datatypes.h"

/**
 *\struct AsyncQueue
 *\brief An asynchronous queue.
 */
typedef struct
{
	/*! Mutex to protect the queue. */
	pthread_mutex_t mutex;
	/*! Condition to wakeup consumers. */
	pthread_cond_t cond;
	/*! The underlying queue. */
	Queue queue;
	/*! Number of waiting consumers. */
	uint32_t waiting;
} AsyncQueue;

/**
 *\param compare function to compare item data
 *\param free function to free item data or NULL
 *\param pool a user-defined memory pool for creating/destroying QueueItems or NULL
 *\return a new AsyncQueue
 *
 * Creates a new AsyncQueue.
 */
AsyncQueue *async_queue_new(CompareFunc compare, FreeFunc free, Pool *pool);

/**
 *\param queue an AsyncQueue
 *\param compare function to compare item data
 *\param free function to free item data or NULL
 *\param pool a user-defined memory pool for creating/destroying QueueItems or NULL
 *
 * Initializes an AsyncQueue.
 */
void async_queue_init(AsyncQueue *queue, CompareFunc compare, FreeFunc free, Pool *pool);

/**
 *\param queue an AsyncQueue
 *
 * Frees all items and the queue pointer.
 */
void async_queue_destroy(AsyncQueue *queue);

/**
 *\param queue an AsyncQueue
 *
 * Frees all items without freeing the queue pointer.
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
 *\param queue an AsyncQueue
 *\return number of items
 *
 * Gets the number of stored items.
 */
size_t async_queue_count(AsyncQueue *queue);

#endif /* __ASYNCQUEUE_H__ */

#endif /* WITH_PTHREAD */

