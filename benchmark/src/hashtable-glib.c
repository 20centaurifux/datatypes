#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <glib.h>

/*
 * compile with:
 * $ gcc hashtable-glib.c -O2 `pkg-config --cflags glib-2.0` `pkg-config --libs glib-2.0` -o hashtable-glib
 */

static void
benchmark(void)
{
	GHashTable *table = g_hash_table_new_full(&g_str_hash, &g_str_equal, &g_free, NULL);

	FILE *fp;

	if((fp = fopen("words.txt", "r")))
	{
		char word[64];

		while(fscanf(fp, "%s", word) > 0)
		{
			void *val;

			if(g_hash_table_lookup_extended(table, word, NULL, &val))
			{
				g_hash_table_replace(table, g_strdup(word), GINT_TO_POINTER(GPOINTER_TO_INT(val) + 1));
			}
			else
			{
				g_hash_table_insert(table, g_strdup(word), GINT_TO_POINTER(1));
			}
		}

		fclose(fp);
	}

	printf("%d\n", g_hash_table_size(table));

	GHashTableIter iter;
	size_t sum = 0;
	void *k, *v;

	g_hash_table_iter_init(&iter, table);

	while(g_hash_table_iter_next(&iter, &k, &v))
	{
		sum += GPOINTER_TO_INT(v);
	}

	printf("%zu\n", sum);

	g_hash_table_destroy(table);
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

