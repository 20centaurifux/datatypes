#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <datatypes.h>

int
main(int argc, char *argv[])
{
	AssocArray *array;
	FILE *fp;
	size_t count = 0;
	size_t v;
	AssocArrayIter iter;
	char word[32];

	array = assoc_array_new(str_compare, &free, NULL);

	assert(array != NULL);

	if((fp = fopen("bible.txt", "r")))
	{
		while(fscanf(fp, "%s", word) > 0)
		{
			if(assoc_array_key_exists(array, word))
			{
				AssocArrayPair *pair = assoc_array_lookup(array, word);
				assoc_array_pair_value(pair)++;
			}
			else
			{
				assoc_array_set(array, strdup(word), (void *)1, true);
				++count;
			}
		}

		assert(count == assoc_array_count(array));

		count = 0;

		assoc_array_iter_init(array, &iter);

		while(assoc_array_iter_next(&iter))
		{
			AssocArrayPair *pair = assoc_array_lookup(array, assoc_array_iter_get_key(&iter));
			v = (size_t)assoc_array_pair_value(pair);

			assert(v == (size_t)assoc_array_iter_get_value(&iter));
			assert(v > 0);

			++count;
		}

		assert(count == assoc_array_count(array));

		assoc_array_remove(array, "she");
		assoc_array_remove(array, "dead");
		assoc_array_remove(array, "those");
		assoc_array_remove(array, "night");
		assoc_array_remove(array, "stone");
		assoc_array_remove(array, "Eris");

		assert(count - 5 == assoc_array_count(array));

		assert(assoc_array_key_exists(array, "she") == false);
		assert(assoc_array_key_exists(array, "dead") == false);
		assert(assoc_array_key_exists(array, "those") == false);
		assert(assoc_array_key_exists(array, "night") == false);
		assert(assoc_array_key_exists(array, "stone") == false);
		assert(assoc_array_key_exists(array, "Eris") == false);
		assert(assoc_array_key_exists(array, "god") == true);

		assoc_array_clear(array);

		assert(assoc_array_count(array) == 0);
		assert(assoc_array_key_exists(array, "god") == false);

		count = 0;

		assoc_array_iter_init(array, &iter);

		while(assoc_array_iter_next(&iter))
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

	// cleanup:
	assoc_array_destroy(array);

	return 0;
}

