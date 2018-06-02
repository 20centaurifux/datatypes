#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>

#include <datatypes/datatypes.h>

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static bool finished = false;

static void *
_worker(void *arg)
{
	AsyncQueue *queue = (AsyncQueue *)arg;
	size_t n;
	size_t sum = 0;

	while(1)
	{
		bool success;

		if(sum % 6)
		{
			success = async_queue_pop_timeout(queue, &n, 1000);
		}
		else
		{
			success = async_queue_pop(queue, &n);
		}

		if(success)
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

	return (void *)sum;
}

int
main(int argc, char *argv[])
{
	AsyncQueue *queues[2];
	pthread_t t[2];
	size_t sum;
	size_t result = 0;
	size_t i = 0;

	for(i = 0; i < 2; ++i)
	{
		queues[i] = async_queue_new(direct_compare, NULL, NULL);
		pthread_create(&t[i], NULL, _worker, queues[i]);
	}

	for(i = 1; i <= 500000; ++i)
	{
		async_queue_push(queues[i % 2], (void *)i);
	}

	pthread_mutex_lock(&mutex);
	finished = true;
	pthread_mutex_unlock(&mutex);

	for(i = 0; i < 2; ++i)
	{
		pthread_join(t[i], (void *)&sum);
		result += sum;
		async_queue_destroy(queues[i]);
	}

	assert(result == 500001);

	return 0;
}

