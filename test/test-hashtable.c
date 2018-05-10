#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <datatypes.h>

int
main(int argc, char *argv[])
{
	HashTable *table;
	FILE *fp;
	size_t count = 0;
	size_t v;
	HashTableIter iter;
	char word[32];

	// initialize table:
	table = hashtable_new(0, &str_hash, &str_equal, &free, NULL);

	assert(table != NULL);

	// read test file:
	if((fp = fopen("bible.txt", "r")))
	{
		// count words:
		while(fscanf(fp, "%s", word) > 0)
		{
			if(hashtable_key_exists(table, word))
			{
				v = (size_t)hashtable_lookup(table, word) + 1;
				hashtable_set(table, (void*)word, (void *)v, false);
			}
			else
			{
				hashtable_set(table, strdup(word), (void *)1, true);
				++count;
			}
		}

		assert(count == hashtable_count(table));

		// iterate:
		count = 0;

		hashtable_iter_init(table, &iter);

		while(hashtable_iter_next(&iter))
		{
			v = (size_t)hashtable_lookup(table, hashtable_iter_get_key(&iter));

			assert(v == (size_t)hashtable_iter_get_value(&iter));
			assert(v > 0);

			++count;
		}

		assert(count == hashtable_count(table));

		// remove keys:
		hashtable_remove(table, "evil");
		hashtable_remove(table, "the");
		hashtable_remove(table, "given");
		hashtable_remove(table, "day");
		hashtable_remove(table, "have");
		hashtable_remove(table, "Eris");

		assert(count - 5 == hashtable_count(table));

		// key lookup:
		assert(hashtable_key_exists(table, "evil") == false);
		assert(hashtable_key_exists(table, "the") == false);
		assert(hashtable_key_exists(table, "given") == false);
		assert(hashtable_key_exists(table, "day") == false);
		assert(hashtable_key_exists(table, "have") == false);
		assert(hashtable_key_exists(table, "Eris") == false);
		assert(hashtable_key_exists(table, "god") == true);

		// clear table:
		hashtable_clear(table);

		assert(hashtable_count(table) == 0);
		assert(hashtable_key_exists(table, "god") == false);

		count = 0;

		hashtable_iter_init(table, &iter);

		while(hashtable_iter_next(&iter))
		{
			++count;
		}

		assert(count == 0);

		// close file:
		fclose(fp);
	}
	else
	{
		fprintf(stderr, "Couldn't read bible.txt");
	}

	// cleanup:
	hashtable_destroy(table);

	return 0;
}

