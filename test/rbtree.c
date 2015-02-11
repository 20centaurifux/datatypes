#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

#include "datatypes.h"
#include "next_word.h"

int
main(int argc, char *argv[])
{
	RBTree *tree;
	FILE *fp;
	const char *word;
	size_t count = 0;
	RBTreeIter iter;

	// initialize tree:
	tree = rbtree_new(str_compare, &free, NULL, NULL);

	assert(tree != NULL);

	// read test file:
	if((fp = fopen("bible.txt", "r")))
	{
		// count words:
		while((word = next_word(fp)))
		{
			if(rbtree_key_exists(tree, word))
			{
				int v = (int)rbtree_lookup(tree, word) + 1;
				rbtree_set(tree, (void*)word, (void *)(intptr_t)v, false);
			}
			else
			{
				rbtree_set(tree, strdup(word), (void *)(intptr_t)1, true);
				++count;
			}
		}

		assert(count == rbtree_count(tree));

		// iterate:
		count = 0;

		rbtree_iter_init(tree, &iter);

		while(rbtree_iter_next(&iter))
		{
			int v = (int)rbtree_lookup(tree, rbtree_iter_get_key(&iter));

			assert(v == (int)rbtree_iter_get_value(&iter));
			assert(v > 0);

			++count;
		}

		assert(count == rbtree_count(tree));

		// remove keys:
		rbtree_remove(tree, "good");
		rbtree_remove(tree, "between");
		rbtree_remove(tree, "father");
		rbtree_remove(tree, "and");
		rbtree_remove(tree, "him");

		assert(count - 5 == rbtree_count(tree));

		// key lookup:
		assert(rbtree_key_exists(tree, "father") == false);
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

		// close file:
		fclose(fp);
	}
	else
	{
		fprintf(stderr, "Couldn't read bible.txt");
	}

	// cleanup:
	rbtree_free(tree);

	return 0;
}

