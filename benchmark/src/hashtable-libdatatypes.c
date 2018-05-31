#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <datatypes.h>

/*
 * compile with:
 * $ gcc -O2 hashtable-libdatatypes.c -I /usr/include/datatypes -o hashtable-libdatatypes /usr/lib64/libdatatypes-0.3.0.a
 */

static void
benchmark(void)
{
	HashTable table;

	hashtable_init(&table, 0, &str_hash, &str_equal, &free, NULL);

	FILE *fp;

	if((fp = fopen("words.txt", "r")))
	{
		char word[64];

		while(fscanf(fp, "%s", word) > 0)
		{
			HashTablePair *pair = hashtable_lookup(&table, word);

			if(pair)
			{
				hashtable_pair_value(pair)++;
			}
			else
			{
				hashtable_set(&table, strdup(word), (void *)1, true);
			}
		}

		fclose(fp);
	}

	printf("%zu\n", hashtable_count(&table));

	HashTableIter iter;
	size_t sum = 0;

	hashtable_iter_init(&table, &iter);

	while(hashtable_iter_next(&iter))
	{
		sum += (size_t)hashtable_iter_value(iter);
	}

	printf("%zu\n", sum);

	hashtable_free(&table);
}

int
main(int argc, char *argv[])
{
	clock_t start = clock();

	benchmark();

	float seconds = (float)(clock() - start) / CLOCKS_PER_SEC;

	printf("%f\n", seconds);

	return 0;
}

