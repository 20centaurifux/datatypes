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
 * \file asyncqueue.c
 * \brief Asynchronous queue.
 * \author Sebastian Fedrau <sebastian.fedrau@gmail.com>
 */
#ifdef WITH_PTHREAD

#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <assert.h>

#include "asyncqueue.h"

AsyncQueue *
async_queue_new(CompareFunc compare, FreeFunc free, Pool *pool)
{
	assert(compare);

	AsyncQueue *queue = (AsyncQueue *)malloc(sizeof(AsyncQueue));

	if(!queue)
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		abort();
	}

	async_queue_init(queue, compare, free, pool);

	return queue;
}

void
async_queue_init(AsyncQueue *queue, CompareFunc compare, FreeFunc free, Pool *pool)
{
	assert(queue != NULL);
	assert(compare != NULL);

	queue_init(&queue->queue, compare, free, pool);
	queue->waiting = 0;

	#ifndef NDEBUG
	int success =
	#endif
	pthread_mutex_init(&queue->mutex, NULL);

	assert(success == 0);

	#ifndef NDEBUG
	success =
	#endif
	pthread_cond_init(&queue->cond, NULL);

	assert(success == 0);
}

void
async_queue_destroy(AsyncQueue *queue)
{
	assert(queue != NULL);

	async_queue_free(queue);
	free(queue);
}

void
async_queue_free(AsyncQueue *queue)
{
	assert(queue != NULL);

	queue_free(&queue->queue);

	pthread_mutex_destroy(&queue->mutex);
	pthread_cond_destroy(&queue->cond);
}

void
async_queue_push(AsyncQueue *queue, void *data)
{
	assert(queue != NULL);

	pthread_mutex_lock(&queue->mutex);

	queue_push(&queue->queue, data);

	if(queue->waiting)
	{
		pthread_cond_signal(&queue->cond);
	}

	pthread_mutex_unlock(&queue->mutex);
}

static bool
_async_queue_pop(AsyncQueue *queue, void *data, uint32_t ms)
{
	assert(queue != NULL);

	pthread_mutex_lock(&queue->mutex);

	bool success = queue_pop(&queue->queue, data);

	if(!success)
	{
		++queue->waiting;

		if(ms)
		{
			struct timespec val;
			struct timeval now;

			gettimeofday(&now, NULL);

			val.tv_sec = now.tv_sec + (ms / 1000);
			val.tv_nsec = 0;

			if(!pthread_cond_timedwait(&queue->cond, &queue->mutex, &val))
			{
				success = queue_pop(&queue->queue, data);
			}
		}
		else
		{
			if(!pthread_cond_wait(&queue->cond, &queue->mutex))
			{
				success = queue_pop(&queue->queue, data);
			}
		}

		--queue->waiting;
	}

	pthread_mutex_unlock(&queue->mutex);

	return success;
}

bool
async_queue_pop(AsyncQueue *queue, void *data)
{
	assert(queue != NULL);

	return _async_queue_pop(queue, data, 0);
}

bool
async_queue_pop_timeout(AsyncQueue *queue, void *data, uint32_t ms)
{
	return _async_queue_pop(queue, data, ms);
}

void
async_queue_clear(AsyncQueue *queue)
{
	assert(queue != NULL);

	pthread_mutex_lock(&queue->mutex);

	queue_clear(&queue->queue);

	pthread_mutex_unlock(&queue->mutex);
}

size_t
async_queue_count(AsyncQueue *queue)
{
	assert(queue != NULL);

	pthread_mutex_lock(&queue->mutex);

	size_t count = queue_count(&queue->queue);

	pthread_mutex_unlock(&queue->mutex);

	return count;
}

#endif /* WITH_PTHREAD */

