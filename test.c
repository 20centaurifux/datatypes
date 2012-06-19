#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "hashtable.h"

#define HASHTABLE_TEST_DATA_SIZE 20000000
#define HASHTABLE_KEY_DATA_SIZE  25000000

char buffer[HASHTABLE_TEST_DATA_SIZE];
char keys[HASHTABLE_KEY_DATA_SIZE];
int count;

HashTable *table;

static void
_test_hashtable(void)
{
	FILE *in;
	char *p = keys;
	void *v;
	int i;
	char key[6];

	count = 0;

	/* create data from test file */
	memset(keys, 0, HASHTABLE_KEY_DATA_SIZE);
	in = fopen("test", "rb");
	fread(buffer, 1, HASHTABLE_TEST_DATA_SIZE, in);
	fclose(in);

	/* insert test data into table (count keys) */
	for(i = 0; i < HASHTABLE_TEST_DATA_SIZE; i += 5)
	{
		memcpy(p, buffer + i, 5);

		if(!(v = hashtable_lookup(table, p)))
		{
			hashtable_set(table, strdup(p), (void *)1, false);
		}
		else
		{
			hashtable_set(table, strdup(p), (void *)((long)v + 1), true);
		}

		p += 6;
	}

	/* remove keys */
	key[5] = '\0';

	for(i = 0; i < HASHTABLE_TEST_DATA_SIZE; i += 5)
	{
		if(!(i % 2))
		{
			strncpy(key, buffer + i, 5);
			hashtable_remove(table, key);
		}
	}

	HashTableIter iter;

	hashtable_iter_init(table, &iter);

	while(hashtable_iter_next(&iter))
	{
		count++;
	}

	printf("%d==%d\n", hashtable_count(table), count);

	hashtable_clear(table);

	printf("%d==%d\n", hashtable_count(table), count);
}

int
main(int argc, char *argv[])
{
	table = hashtable_new(10000000, str_hash, str_equal, free, NULL);

	_test_hashtable();
	_test_hashtable();
	_test_hashtable();

	printf("destroy\n");
	hashtable_destroy(table);

	return 0;
}

