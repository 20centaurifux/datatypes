#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <datatypes.h>

int
main(int argc, char *argv[])
{
	RBTree *tree;
	FILE *fp;
	size_t count = 0;
	size_t v;
	RBTreeIter iter;
	char word[32];

	// initialize tree:
	tree = rbtree_new(str_compare, &free, NULL, NULL);

	assert(tree != NULL);

	// read test file:
	if((fp = fopen("bible.txt", "r")))
	{
		// count words:
		while(fscanf(fp, "%s", word) > 0)
		{
			if(rbtree_key_exists(tree, word))
			{
				v = (size_t)rbtree_lookup(tree, word) + 1;
				rbtree_set(tree, (void*)word, (void *)v, false);
			}
			else
			{
				rbtree_set(tree, strdup(word), (void *)1, true);
				++count;
			}
		}

		assert(count == rbtree_count(tree));

		// iterate:
		count = 0;

		rbtree_iter_init(tree, &iter);

		while(rbtree_iter_next(&iter))
		{
			v = (size_t)rbtree_lookup(tree, rbtree_iter_get_key(&iter));

			assert(v == (size_t)rbtree_iter_get_value(&iter));
			assert(v > 0);

			++count;
		}

		assert(count == rbtree_count(tree));

		// remove keys:
		rbtree_remove(tree, "she");
		rbtree_remove(tree, "dead");
		rbtree_remove(tree, "those");
		rbtree_remove(tree, "night");
		rbtree_remove(tree, "stone");
		rbtree_remove(tree, "Eris");

		assert(count - 5 == rbtree_count(tree));

		// key lookup:
		assert(rbtree_key_exists(tree, "she") == false);
		assert(rbtree_key_exists(tree, "dead") == false);
		assert(rbtree_key_exists(tree, "those") == false);
		assert(rbtree_key_exists(tree, "night") == false);
		assert(rbtree_key_exists(tree, "stone") == false);
		assert(rbtree_key_exists(tree, "Eris") == false);
		assert(rbtree_key_exists(tree, "god") == true);

		// clear tree:
		rbtree_clear(tree);

		assert(rbtree_count(tree) == 0);
		assert(rbtree_key_exists(tree, "god") == false);

		count = 0;

		rbtree_iter_init(tree, &iter);

		while(rbtree_iter_next(&iter))
		{
			++count;
		}

		assert(count == 0);

		rbtree_iter_free(&iter);

		// close file:
		fclose(fp);
	}
	else
	{
		fprintf(stderr, "Couldn't read bible.txt");
	}

	// cleanup:
	rbtree_destroy(tree);

	return 0;
}

