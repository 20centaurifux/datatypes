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
	TYPENAME *map = PREFIX`_new'(NEW_STR_ARGS);

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

		ifdef(`HAS_ITER_FREE_FUNC', PREFIX`_iter_free'(&iter);)

		assert(count == PREFIX`_count'(map));

		fclose(fp);
	}

	PREFIX`_destroy'(map);
}

static void
test_count_words_without_macros(void)
{
	TYPENAME *map = PREFIX`_new'(NEW_STR_ARGS);

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

		ifdef(`HAS_ITER_FREE_FUNC', PREFIX`_iter_free'(&iter);)

		assert(count == PREFIX`_count'(map));

		fclose(fp);
	}

	PREFIX`_destroy'(map);
}

static void
test_remove_and_clear(void)
{
	TYPENAME *map = PREFIX`_new'(NEW_STR_ARGS);

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

#define INT_TO_PTR(i) (void *)(intptr_t)i

static void
test_int(void)
{
	TYPENAME *map = PREFIX`_new'(NEW_INT_ARGS);

	assert(map != NULL);

	int total = 0;

	for(int32_t i = 1; i <= 10000; ++i)
	{
		if(i % 3)
		{
			if(i % 2)
			{
				total += i * 2;
			}
			else
			{
				total += i;
			}
		}
	}

	for(int32_t i = 1; i <= 10000; ++i)
	{
		PREFIX`_set'(map, INT_TO_PTR(i), INT_TO_PTR(i), false);
	}

	for(int32_t i = 1; i <= 9999; i += 2)
	{
		TYPENAME`Pair' *pair = PREFIX`_lookup'(map, INT_TO_PTR(i));

		int v = (intptr_t)PREFIX`_pair_get_value'(pair) * 2;
		PREFIX`_set'(map, INT_TO_PTR(i), INT_TO_PTR(v), false);
	}

	for(int32_t i = 3; i <= 9999; i += 3)
	{
		PREFIX`_remove'(map, INT_TO_PTR(i));
	}

	TYPENAME`Iter' iter;
	int total_iter = 0;

	PREFIX`_iter_init'(map, &iter);

	while(PREFIX`_iter_next'(&iter))
	{
		total_iter += (intptr_t)PREFIX`_iter_value'(iter);
	}

	ifdef(`HAS_ITER_FREE_FUNC', PREFIX`_iter_free'(&iter);)

	assert(total == total_iter);

	PREFIX`_destroy'(map);
}

int
main(int argc, char *argv[])
{
	test_count_words_with_macros();
	test_count_words_without_macros();
	test_remove_and_clear();
	test_int();

	return 0;
}

