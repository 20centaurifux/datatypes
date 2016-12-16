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

	// initialize array:
	array = assoc_array_new(str_compare, &free, NULL);

	assert(array != NULL);

	// read test file:
	if((fp = fopen("bible.txt", "r")))
	{
		// count words:
		while(fscanf(fp, "%s", word) > 0)
		{
			if(assoc_array_key_exists(array, word))
			{
				v = (size_t)assoc_array_lookup(array, word) + 1;
				assoc_array_set(array, (void*)word, (void *)v, false);
			}
			else
			{
				assoc_array_set(array, strdup(word), (void *)1, true);
				++count;
			}
		}

		assert(count == assoc_array_count(array));

		// iterate:
		count = 0;

		assoc_array_iter_init(array, &iter);

		while(assoc_array_iter_next(&iter))
		{
			v = (size_t)assoc_array_lookup(array, assoc_array_iter_get_key(&iter));

			assert(v == (size_t)assoc_array_iter_get_value(&iter));
			assert(v > 0);

			++count;
		}

		assert(count == assoc_array_count(array));

		// remove keys:
		assoc_array_remove(array, "she");
		assoc_array_remove(array, "dead");
		assoc_array_remove(array, "those");
		assoc_array_remove(array, "night");
		assoc_array_remove(array, "stone");
		assoc_array_remove(array, "Eris");

		assert(count - 5 == assoc_array_count(array));

		// key lookup:
		assert(assoc_array_key_exists(array, "she") == false);
		assert(assoc_array_key_exists(array, "dead") == false);
		assert(assoc_array_key_exists(array, "those") == false);
		assert(assoc_array_key_exists(array, "night") == false);
		assert(assoc_array_key_exists(array, "stone") == false);
		assert(assoc_array_key_exists(array, "Eris") == false);
		assert(assoc_array_key_exists(array, "god") == true);

		// clear array:
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

		// close file:
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

