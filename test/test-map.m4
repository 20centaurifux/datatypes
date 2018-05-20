#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#
#include <datatypes/datatypes.h>

static void
test_count_words_with_macros(void)
{
	TYPENAME *map = PREFIX`_new'(NEW_ARGS);

	assert(map != NULL);

	FILE *fp;

	if((fp = fopen("bible.txt", "r")))
	{
		char word[32];
		size_t count = 0;

		while(fscanf(fp, "%s", word) > 0)
		{
			if(PREFIX`_key_exists'(map, word))
			{
				TYPENAME`Pair' *pair = PREFIX`_lookup'(map, word);

				assert(pair != NULL);
				assert(strcmp(PREFIX`_pair_key'(pair), word) == 0);

				PREFIX`_pair_value'(pair)++;
			}
			else
			{
				TYPENAME`Pair' *pair = PREFIX`_lookup'(map, word);

				assert(pair == NULL);

				PREFIX`_set'(map, strdup(word), (void *)1, true);
				++count;
			}
		}

		assert(count == PREFIX`_count'(map));

		count = 0;

		TYPENAME`Iter' iter;

		PREFIX`_iter_init'(map, &iter);

		while(PREFIX`_iter_next'(&iter))
		{
			TYPENAME`Pair' *pair = PREFIX`_lookup'(map, PREFIX`_iter_key'(iter));
			size_t v = (size_t)PREFIX`_pair_value'(pair);

			assert(v == (size_t)PREFIX`_iter_value'(iter));
			assert(v > 0);

			++count;
		}

		assert(count == PREFIX`_count'(map));

		fclose(fp);
	}

	PREFIX`_destroy'(map);
}

static void
test_count_words_without_macros(void)
{
	TYPENAME *map = PREFIX`_new'(NEW_ARGS);

	assert(map != NULL);

	FILE *fp;

	if((fp = fopen("bible.txt", "r")))
	{
		char word[32];
		size_t count = 0;

		while(fscanf(fp, "%s", word) > 0)
		{
			if(PREFIX`_key_exists'(map, word))
			{
				TYPENAME`Pair' *pair = PREFIX`_lookup'(map, word);

				assert(pair != NULL);
				assert(strcmp(PREFIX`_pair_get_key'(pair), word) == 0);

				size_t v = (size_t)PREFIX`_pair_get_value'(pair) + 1;

				PREFIX`_pair_set_value'(pair, (void *)v);
			}
			else
			{
				TYPENAME`Pair' *pair = PREFIX`_lookup'(map, word);

				assert(pair == NULL);

				PREFIX`_set'(map, strdup(word), (void *)1, true);
				++count;
			}
		}

		assert(count == PREFIX`_count'(map));

		count = 0;

		TYPENAME`Iter' iter;

		PREFIX`_iter_init'(map, &iter);

		while(PREFIX`_iter_next'(&iter))
		{
			TYPENAME`Pair' *pair = PREFIX`_lookup'(map, PREFIX`_iter_get_key'(&iter));
			size_t v = (size_t)PREFIX`_pair_get_value'(pair);

			assert(v == (size_t)PREFIX`_iter_get_value'(&iter));
			assert(v > 0);

			++count;
		}

		assert(count == PREFIX`_count'(map));

		fclose(fp);
	}

	PREFIX`_destroy'(map);
}

static void
test_remove_and_clear(void)
{
	TYPENAME *map = PREFIX`_new'(NEW_ARGS);

	assert(map != NULL);

	FILE *fp;

	if((fp = fopen("bible.txt", "r")))
	{
		char word[32];
		size_t count = 0;

		while(fscanf(fp, "%s", word) > 0)
		{
			if(PREFIX`_key_exists'(map, word))
			{
				TYPENAME`Pair' *pair = PREFIX`_lookup'(map, word);

				size_t v = (size_t)PREFIX`_pair_get_value'(pair) + 1;

				PREFIX`_set'(map, word, (void *)v, false);
			}
			else
			{
				PREFIX`_set'(map, strdup(word), (void *)1, true);
				++count;
			}
		}

		PREFIX`_remove'(map, "she");
		PREFIX`_remove'(map, "dead");
		PREFIX`_remove'(map, "those");
		PREFIX`_remove'(map, "night");
		PREFIX`_remove'(map, "stone");
		PREFIX`_remove'(map, "Eris");

		assert(count - 5 == PREFIX`_count'(map));

		assert(PREFIX`_key_exists'(map, "she") == false);
		assert(PREFIX`_key_exists'(map, "dead") == false);
		assert(PREFIX`_key_exists'(map, "those") == false);
		assert(PREFIX`_key_exists'(map, "night") == false);
		assert(PREFIX`_key_exists'(map, "stone") == false);
		assert(PREFIX`_key_exists'(map, "Eris") == false);
		assert(PREFIX`_key_exists'(map, "god") == true);

		PREFIX`_clear'(map);

		assert(PREFIX`_count'(map) == 0);
		assert(PREFIX`_key_exists'(map, "god") == false);

		count = 0;

		TYPENAME`Iter' iter;

		PREFIX`_iter_init'(map, &iter);

		while(PREFIX`_iter_next'(&iter))
		{
			++count;
		}

		assert(count == 0);

		fclose(fp);
	}

	PREFIX`_destroy'(map);
}

int
main(int argc, char *argv[])
{
	test_count_words_with_macros();
	test_count_words_without_macros();
	test_remove_and_clear();

	return 0;
}

