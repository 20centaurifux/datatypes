#include "datatypes.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void *
_worker(void *arg)
{
	AsyncQueue *queue = (AsyncQueue *)arg;

	printf("worker started\n");

	int foo;

	if(async_queue_pop_timeout((AsyncQueue *)queue, &foo, 4000))
	{
		printf("received data: %d\n", foo);
	}
	else
	{
		printf("timeout\n");
	}

	return NULL;
}

int
main(int argc, char *argv[])
{
	AsyncQueue *queue;
	pthread_t t;

	queue = async_queue_new(direct_equal, NULL, NULL);

	pthread_create(&t, NULL, _worker, queue);

	usleep(5000000);

	async_queue_push(queue, (void *)23);

	pthread_join(t, NULL);

	async_queue_free(queue);


	return 0;
}

