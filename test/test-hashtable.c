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
	HashTableIter iter;
	char word[32];

	table = hashtable_new(0, &str_hash, &str_equal, &free, NULL);

	assert(table != NULL);

	if((fp = fopen("bible.txt", "r")))
	{
		count = 0;

		while(fscanf(fp, "%s", word) > 0)
		{
			if(hashtable_key_exists(table, word))
			{
				HashTablePair *pair = hashtable_lookup(table, word);

				assert(pair != NULL);

				assert(strcmp(hashtable_pair_get_key(pair), word) == 0);

				size_t v = (size_t)hashtable_pair_get_value(pair) + 1;

				hashtable_pair_set_value(pair, (void *)v);
			}
			else
			{
				HashTablePair *pair = hashtable_lookup(table, word);

				assert(pair == NULL);

				hashtable_set(table, strdup(word), (void *)1, true);
				++count;
			}
		}

		assert(count == hashtable_count(table));

		count = 0;

		hashtable_iter_init(table, &iter);

		while(hashtable_iter_next(&iter))
		{
			HashTablePair *pair = hashtable_lookup(table, hashtable_iter_get_key(&iter));

			assert(pair != NULL);

			size_t v = (size_t)hashtable_pair_get_value(pair);

			assert(v == (size_t)hashtable_iter_get_value(&iter));
			assert(v > 0);

			++count;
		}

		assert(count == hashtable_count(table));

		hashtable_remove(table, "evil");
		hashtable_remove(table, "the");
		hashtable_remove(table, "given");
		hashtable_remove(table, "day");
		hashtable_remove(table, "have");
		hashtable_remove(table, "Eris");

		assert(count - 5 == hashtable_count(table));

		assert(hashtable_key_exists(table, "evil") == false);
		assert(hashtable_key_exists(table, "the") == false);
		assert(hashtable_key_exists(table, "given") == false);
		assert(hashtable_key_exists(table, "day") == false);
		assert(hashtable_key_exists(table, "have") == false);
		assert(hashtable_key_exists(table, "Eris") == false);
		assert(hashtable_key_exists(table, "god") == true);

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

		fclose(fp);
	}
	else
	{
		fprintf(stderr, "Couldn't read bible.txt");
	}

	hashtable_destroy(table);

	return 0;
}

