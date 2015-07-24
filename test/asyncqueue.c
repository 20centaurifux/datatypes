#include "datatypes.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static bool finished = false;

static void *
_worker(void *arg)
{
	AsyncQueue *queue = (AsyncQueue *)arg;
	uint32_t n;
	uint32_t sum = 0;

	while(1)
	{
		if(async_queue_pop_timeout(queue, &n, 10))
		{
			sum += n % 3;
		}
		else
		{
			bool cancelled;

			pthread_mutex_lock(&mutex);
			cancelled = finished;
			pthread_mutex_unlock(&mutex);

			if(cancelled)
			{
				break;
			}
		}
	}

	pthread_exit((void *)sum);

	return NULL;
}

int
main(int argc, char *argv[])
{
	AsyncQueue queues[2];
	pthread_t t[2];
	uint32_t sum;
	uint32_t result = 0;
	uint32_t i = 0;

	for(i = 0; i < 2; ++i)
	{
		async_queue_init(&queues[i], direct_equal, NULL, NULL);
		pthread_create(&t[i], NULL, _worker, &queues[i]);
	}

	for(i = 1; i <= 5000000; ++i)
	{
		async_queue_push(&queues[i % 2], i);
	}

	pthread_mutex_lock(&mutex);

	finished = true;

	pthread_mutex_unlock(&mutex);

	for(i = 0; i < 2; ++i)
	{
		pthread_join(t[i], &sum);
		result += sum;
		async_queue_destroy(&queues[i]);
	}

	printf("result: %d\n", result);

	return 0;
}

