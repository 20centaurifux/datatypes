#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <uthash.h>

typedef struct {
	char word[64];
	size_t count;
	UT_hash_handle hh;
} dictionary;

static void
benchmark(void)
{
	dictionary *dict = NULL;

	FILE *fp;

	if((fp = fopen("words.txt", "r")))
	{
		char word[64];

		while(fscanf(fp, "%s", word) > 0)
		{
			dictionary *found = NULL;

			HASH_FIND_STR(dict, word, found);
			if (found)
			{
				found->count++;
			}
			else
			{
				found = malloc(sizeof(dictionary));
				strcpy(found->word, word);
				found->count = 1;
				HASH_ADD_STR(dict, word, found);
			}
		}

		fclose(fp);
	}

	printf("%d\n", HASH_COUNT(dict));

	size_t sum = 0;
	dictionary *iter = NULL, *tmp;

	HASH_ITER(hh, dict, iter, tmp)
	{
		sum += iter->count;
		HASH_DEL(dict, iter);
		free(iter);
	}

	printf("%zu\n", sum);
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

