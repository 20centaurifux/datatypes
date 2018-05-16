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
	RBTreeIter iter;
	char word[32];

	tree = rbtree_new(str_compare, &free, NULL, NULL);

	assert(tree != NULL);

	if((fp = fopen("bible.txt", "r")))
	{
		while(fscanf(fp, "%s", word) > 0)
		{
			RBTreePair *pair = rbtree_lookup(tree, word);

			if(pair)
			{
				size_t v = (size_t)rbtree_pair_get_value(pair) + 1;
				rbtree_set(tree, word, (void *)v, false);
			}
			else
			{
				rbtree_set(tree, strdup(word), (void *)1, true);
				++count;
			}
		}

		assert(count == rbtree_count(tree));

		count = 0;

		rbtree_iter_init(tree, &iter);

		while(rbtree_iter_next(&iter))
		{
			RBTreePair *pair = rbtree_lookup(tree, rbtree_iter_get_key(&iter));
			size_t v = (size_t)rbtree_pair_value(pair);

			assert(v == (size_t)rbtree_iter_get_value(&iter));
			assert(v > 0);

			++count;
		}

		assert(count == rbtree_count(tree));

		rbtree_remove(tree, "she");
		rbtree_remove(tree, "dead");
		rbtree_remove(tree, "those");
		rbtree_remove(tree, "night");
		rbtree_remove(tree, "stone");
		rbtree_remove(tree, "Eris");

		assert(count - 5 == rbtree_count(tree));

		assert(rbtree_key_exists(tree, "she") == false);
		assert(rbtree_key_exists(tree, "dead") == false);
		assert(rbtree_key_exists(tree, "those") == false);
		assert(rbtree_key_exists(tree, "night") == false);
		assert(rbtree_key_exists(tree, "stone") == false);
		assert(rbtree_key_exists(tree, "Eris") == false);
		assert(rbtree_key_exists(tree, "god") == true);

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

		fclose(fp);
	}
	else
	{
		fprintf(stderr, "Couldn't read bible.txt");
	}

	rbtree_destroy(tree);

	return 0;
}

