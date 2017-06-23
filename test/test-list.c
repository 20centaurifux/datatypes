#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <datatypes.h>

int
main(int argc, char *argv[])
{
	List *list;
	FILE *fp;
	char word[64];
	size_t count = 0, god_count = 0;
	ListItem *item, *prev;
	char *ptr;

	// initialize lists:
	list = list_new(str_compare, &free, NULL);

	assert(list != NULL);
	assert(list_empty(list) == true);

	// put words from bible into list:
	if((fp = fopen("bible.txt", "r")))
	{
		// count words:
		while(fscanf(fp, "%s", word) > 0)
		{
			list_append(list, strdup(word));
			++count;
		}

		fclose(fp);
	}

	assert(list_count(list) == count);
	assert(list_empty(list) == false);

	// iterate through list:
	item = list_head(list);

	while(item)
	{
		--count;
		item = list_item_next(item);
	}

	assert(count == 0);

	item = list_tail(list);

	while(item)
	{
		++count;
		item = list_item_prev(item);
	}

	assert(count == list_count(list));

	// remove word:
	item = list_head(list);

	while(item)
	{
		if(!strcmp(list_item_get_data(item), "God"))
		{
			++god_count;
		}

		item = list_item_next(item);
	}

	list_remove_by_data(list, "God", true);

	assert(list_count(list) == count - god_count);

	// lookup & replace words:
	item = list_find(list, NULL, (void *)"God");

	assert(item == NULL);
	assert(list_contains(list, "God") == false);

	item = list_find(list, NULL, (void *)"she");

	assert(item != NULL);
	assert(!strcmp("she", (char *)list_item_get_data(item)));

	list_item_free_data(list, item);
	list_item_set_data(item, strdup("Eris"));

	item = list_find(list, item, (void *)"she");

	assert(item != NULL);
	assert(!strcmp("she", (char *)list_item_get_data(item)));

	item = list_find(list, NULL, (void *)"Eris");

	assert(!strcmp("Eris", (char *)list_item_get_data(item)));
	assert(list_contains(list, "Eris") == true);

	// clear list:
	list_clear(list);

	assert(list_empty(list) == true);
	assert(list_count(list) == 0);

	// insert sorted:
	list_insert_sorted(list, strdup("d"));
	list_insert_sorted(list, strdup("b"));
	list_insert_sorted(list, strdup("c"));
	
	item = list_prepend(list, strdup("e"));
	list_reorder(list, item);

	item = list_append(list, strdup("a"));
	list_reorder(list, item);

	item = list_head(list);
	prev = NULL;

	while(item)
	{
		if(prev)
		{
			assert(strcmp((char *)list_item_get_data(prev), (char *)list_item_get_data(item)) < 0);
		}

		prev = item;
		item = list_item_next(item);
	}

	// pop:
	count = 0;

	while((ptr = list_pop(list)))
	{
		++count;
		free(ptr);
	}

	assert(count == 5);

	list_destroy(list);
}

