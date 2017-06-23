#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <datatypes.h>

int
main(int argc, char *argv[])
{
	SList *list;
	FILE *fp;
	char word[64];
	size_t count = 0, god_count = 0;
	SListItem *item, *prev;
	char *ptr;

	// initialize lists:
	list = slist_new(str_compare, &free, NULL);

	assert(list != NULL);
	assert(slist_empty(list) == true);

	// put words from bible into list:
	if((fp = fopen("bible.txt", "r")))
	{
		// count words:
		while(fscanf(fp, "%s", word) > 0)
		{
			slist_append(list, strdup(word));
			++count;
		}
	}

	assert(slist_count(list) == count);
	assert(slist_empty(list) == false);

	// iterate through list:
	item = slist_head(list);
	count = 0;

	while(item)
	{
		++count;
		item = list_item_next(item);
	}

	assert(count == slist_count(list));

	// remove word:
	item = slist_head(list);

	while(item)
	{
		if(!strcmp(list_item_get_data(item), "God"))
		{
			++god_count;
		}

		item = list_item_next(item);
	}

	slist_remove_by_data(list, "God", true);

	assert(slist_count(list) == count - god_count);

	// lookup & replace words:
	item = slist_find(list, NULL, (void *)"God");

	assert(item == NULL);
	assert(slist_contains(list, "God") == false);

	item = slist_find(list, NULL, (void *)"she");

	assert(item != NULL);
	assert(!strcmp("she", (char *)list_item_get_data(item)));

	slist_item_free_data(list, item);
	slist_item_set_data(item, strdup("Eris"));

	item = slist_find(list, item, (void *)"she");

	assert(item != NULL);
	assert(!strcmp("she", (char *)slist_item_get_data(item)));

	item = slist_find(list, NULL, (void *)"Eris");

	assert(!strcmp("Eris", (char *)slist_item_get_data(item)));
	assert(slist_contains(list, "Eris") == true);

	// clear list:
	slist_clear(list);

	assert(slist_empty(list) == true);
	assert(slist_count(list) == 0);

	// insert sorted:
	slist_insert_sorted(list, strdup("d"));
	slist_insert_sorted(list, strdup("b"));
	slist_insert_sorted(list, strdup("c"));
	
	item = slist_prepend(list, strdup("e"));
	slist_reorder(list, item);

	item = slist_append(list, strdup("a"));
	slist_reorder(list, item);

	item = slist_head(list);
	prev = NULL;

	while(item)
	{
		if(prev)
		{
			assert(strcmp((char *)slist_item_get_data(prev), (char *)slist_item_get_data(item)) < 0);
		}

		prev = item;
		item = slist_item_next(item);
	}

	// pop:
	count = 0;

	while((ptr = slist_pop(list)))
	{
		++count;
		free(ptr);
	}

	assert(count == 5);

	slist_destroy(list);
}


