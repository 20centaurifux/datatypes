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

static bool
_foreach_count(void *key, void *value, void *user_data)
{
	++count;

	return true;
}

static void
_test_hashtable(void)
{
	FILE *in;
	HashTable *table;
	char *p = keys;
	void *v;
	int i;
	char key[6];

	/* create data from test file */
	memset(keys, 0, HASHTABLE_KEY_DATA_SIZE);
	in = fopen("test", "rb");
	fread(buffer, 1, HASHTABLE_TEST_DATA_SIZE, in);
	fclose(in);

	/* create hashtable */
	table = hashtable_new(10000000, str_hash, str_compare, NULL, NULL);

	/* insert test data into table (count keys) */
	for(i = 0; i < HASHTABLE_TEST_DATA_SIZE; i += 5)
	{
		memcpy(p, buffer + i, 5);

		if(!(v = hashtable_lookup(table, p)))
		{
			hashtable_set(table, p, (void *)1, false);
		}
		else
		{
			hashtable_set(table, p, (void *)((long)v + 1), false);
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

	/* foreach */
	count = 0;
	hashtable_foreach(table, _foreach_count, NULL);

	printf("%d==%d", hashtable_count(table), count);

	/* cleanup */
	hashtable_destroy(table);
}

int
main(int argc, char *argv[])
{
	_test_hashtable();

	return 0;
}

