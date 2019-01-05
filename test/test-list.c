#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <datatypes/datatypes.h>

static void
_test_new(void)
{
	List *list = list_new(str_compare, free, NULL);

	assert(list->count == 0);
	assert(list->head == NULL);
	assert(list->tail == NULL);
	assert(list->compare == str_compare);
	assert(list->free == free);
	assert(list->pool == NULL);

	list_destroy(list);
}

static void
_test_empty(Pool *pool)
{
	List *list = list_new(str_compare, free, pool);

	assert(list_empty(list));

	list_append(list, strdup("a"));

	assert(!list_empty(list));

	list_destroy(list);
}

static void
_test_count(Pool *pool)
{
	List *list = list_new(str_compare, free, pool);

	assert(list->count == 0);
	assert(list->count == list_count(list));

	list_append(list, strdup("a"));

	assert(list->count == 1);
	assert(list->count == list_count(list));

	list_destroy(list);
}

static void
_test_head(Pool *pool)
{
	List *list = list_new(str_compare, free, pool);

	assert(list->head == NULL);
	assert(list->head == list_head(list));

	ListItem *a = list_append(list, strdup("a"));

	assert(list->head == a);
	assert(list->head == list_head(list));

	list_destroy(list);
}

static void
_test_tail(Pool *pool)
{
	List *list = list_new(str_compare, free, pool);

	assert(list->tail == NULL);
	assert(list->tail == list_tail(list));

	list_append(list, strdup("a"));
	ListItem *b = list_append(list, strdup("b"));

	assert(list->tail == b);
	assert(list->tail == list_tail(list));

	list_destroy(list);
}

static void
_test_prepend(Pool *pool)
{
	List *list = list_new(str_compare, free, pool);

	ListItem *a = list_prepend(list, strdup("a"));

	assert(list->count == 1);
	assert(list->head == a);
	assert(list->tail == a);

	ListItem *b = list_prepend(list, strdup("b"));

	assert(list->count == 2);
	assert(list->head == b);
	assert(list->tail == a);

	assert(!strcmp(list->head->data, "b"));
	assert(list->head->prev == NULL);

	assert(!strcmp(list->head->next->data, "a"));
	assert(!strcmp(list->head->next->prev->data, "b"));

	assert(list->head->next->next == NULL);

	list_destroy(list);
}

static void
_test_append(Pool *pool)
{
	List *list = list_new(str_compare, free, pool);

	ListItem *a = list_append(list, strdup("a"));

	assert(list->count == 1);
	assert(list->head == a);
	assert(list->tail == a);

	ListItem *b = list_append(list, strdup("b"));

	assert(list->count == 2);
	assert(list->head == a);
	assert(list->tail == b);

	assert(!strcmp(list->head->data, "a"));
	assert(list->head->prev == NULL);

	assert(!strcmp(list->head->next->data, "b"));
	assert(!strcmp(list->head->next->prev->data, "a"));

	assert(list->head->next->next == NULL);

	list_destroy(list);
}

static void
_test_sorted(Pool *pool)
{
	List *list = list_new(str_compare, free, pool);

	ListItem *c = list_insert_sorted(list, strdup("c"));

	assert(list->count == 1);
	assert(list->head == c);
	assert(list->tail == c);

	ListItem *b = list_insert_sorted(list, strdup("b"));

	assert(list->count == 2);
	assert(list->head == b);
	assert(list->tail == c);

	ListItem *a = list_insert_sorted(list, strdup("a"));

	assert(list->count == 3);
	assert(list->head == a);
	assert(list->tail == c);

	assert(!strcmp(list->head->data, "a"));
	assert(list->head->prev == NULL);

	assert(!strcmp(list->head->next->data, "b"));
	assert(!strcmp(list->head->next->prev->data, "a"));

	assert(!strcmp(list->head->next->next->data, "c"));
	assert(!strcmp(list->head->next->next->prev->data, "b"));

	assert(list->head->next->next->next == NULL);

	list_destroy(list);
}

static void
_test_remove_head(Pool *pool)
{
	List *list = list_new(str_compare, free, pool);

	ListItem *a = list_append(list, strdup("a"));
	ListItem *b = list_append(list, strdup("b"));
	ListItem *c = list_append(list, strdup("c"));

	list_remove(list, a);

	assert(list->count == 2);
	assert(list->head == b);
	assert(list->tail == c);

	assert(!strcmp(list->head->data, "b"));
	assert(list->head->prev == NULL);

	assert(!strcmp(list->head->next->data, "c"));
	assert(!strcmp(list->head->next->prev->data, "b"));

	assert(list->head->next->next == NULL);

	list_destroy(list);
}

static void
_test_remove_middle(Pool *pool)
{
	List *list = list_new(str_compare, free, pool);

	ListItem *a = list_append(list, strdup("a"));
	ListItem *b = list_append(list, strdup("b"));
	ListItem *c = list_append(list, strdup("c"));

	list_remove(list, b);

	assert(list->count == 2);
	assert(list->head == a);
	assert(list->tail == c);

	assert(!strcmp(list->head->data, "a"));
	assert(list->head->prev == NULL);

	assert(!strcmp(list->head->next->data, "c"));
	assert(!strcmp(list->head->next->prev->data, "a"));

	assert(list->head->next->next == NULL);

	list_destroy(list);
}

static void
_test_remove_tail(Pool *pool)
{
	List *list = list_new(str_compare, free, pool);

	ListItem *a = list_append(list, strdup("a"));
	ListItem *b = list_append(list, strdup("b"));
	ListItem *c = list_append(list, strdup("c"));

	list_remove(list, c);

	assert(list->count == 2);
	assert(list->head == a);
	assert(list->tail == b);

	assert(!strcmp(list->head->data, "a"));
	assert(list->head->prev == NULL);

	assert(!strcmp(list->head->next->data, "b"));
	assert(!strcmp(list->head->next->prev->data, "a"));

	assert(list->head->next->next == NULL);

	list_destroy(list);
}

static void
_test_remove_head_by_data(Pool *pool)
{
	List *list = list_new(str_compare, free, pool);

	list_append(list, strdup("a"));
	ListItem *b = list_append(list, strdup("b"));
	ListItem *c = list_append(list, strdup("c"));

	list_remove_by_data(list, "a", false);

	assert(list->count == 2);
	assert(list->head == b);
	assert(list->tail == c);

	assert(!strcmp(list->head->data, "b"));
	assert(list->head->prev == NULL);

	assert(!strcmp(list->head->next->data, "c"));
	assert(!strcmp(list->head->next->prev->data, "b"));

	assert(list->head->next->next == NULL);

	list_destroy(list);
}

static void
_test_remove_middle_by_data(Pool *pool)
{
	List *list = list_new(str_compare, free, pool);

	ListItem *a = list_append(list, strdup("a"));
	list_append(list, strdup("b"));
	ListItem *c = list_append(list, strdup("c"));

	list_remove_by_data(list, "b", false);

	assert(list->count == 2);
	assert(list->head == a);
	assert(list->tail == c);

	assert(!strcmp(list->head->data, "a"));
	assert(list->head->prev == NULL);

	assert(!strcmp(list->head->next->data, "c"));
	assert(!strcmp(list->head->next->prev->data, "a"));

	assert(list->head->next->next == NULL);

	list_destroy(list);
}

static void
_test_remove_tail_by_data(Pool *pool)
{
	List *list = list_new(str_compare, free, pool);

	ListItem *a = list_append(list, strdup("a"));
	ListItem *b = list_append(list, strdup("b"));
	list_append(list, strdup("c"));

	list_remove_by_data(list, "c", false);

	assert(list->count == 2);
	assert(list->head == a);
	assert(list->tail == b);

	assert(!strcmp(list->head->data, "a"));
	assert(list->head->prev == NULL);

	assert(!strcmp(list->head->next->data, "b"));
	assert(!strcmp(list->head->next->prev->data, "a"));

	assert(list->head->next->next == NULL);

	list_destroy(list);
}

static void
_test_remove_all_data_with_head(Pool *pool)
{
	List *list = list_new(str_compare, free, pool);

	list_append(list, strdup("a"));
	list_append(list, strdup("b"));
	list_append(list, strdup("b"));
	list_append(list, strdup("b"));
	list_append(list, strdup("a"));
	list_append(list, strdup("c"));

	list_remove_by_data(list, "a", true);

	assert(list->count == 4);

	assert(!strcmp(list->head->data, "b"));
	assert(list->head->prev == NULL);

	assert(!strcmp(list->head->next->data, "b"));
	assert(!strcmp(list->head->next->prev->data, "b"));

	assert(!strcmp(list->head->next->next->data, "b"));
	assert(!strcmp(list->head->next->next->prev->data, "b"));

	assert(!strcmp(list->head->next->next->next->data, "c"));
	assert(!strcmp(list->head->next->next->next->prev->data, "b"));

	assert(list->head->next->next->next->next == NULL);

	list_destroy(list);
}

static void
_test_remove_all_data_in_middle(Pool *pool)
{
	List *list = list_new(str_compare, free, pool);

	list_append(list, strdup("a"));
	list_append(list, strdup("b"));
	list_append(list, strdup("b"));
	list_append(list, strdup("b"));
	list_append(list, strdup("c"));

	list_remove_by_data(list, "b", true);

	assert(list->count == 2);

	assert(!strcmp(list->head->data, "a"));
	assert(list->head->prev == NULL);

	assert(!strcmp(list->head->next->data, "c"));
	assert(!strcmp(list->head->next->prev->data, "a"));

	assert(list->head->next->next == NULL);

	list_destroy(list);
}

static void
_test_remove_all_data_with_tail(Pool *pool)
{
	List *list = list_new(str_compare, free, pool);

	list_append(list, strdup("c"));
	list_append(list, strdup("b"));
	list_append(list, strdup("b"));
	list_append(list, strdup("b"));
	list_append(list, strdup("a"));
	list_append(list, strdup("a"));

	list_remove_by_data(list, "a", true);

	assert(list->count == 4);

	assert(!strcmp(list->head->data, "c"));
	assert(list->head->prev == NULL);

	assert(!strcmp(list->head->next->data, "b"));
	assert(!strcmp(list->head->next->prev->data, "c"));

	assert(!strcmp(list->head->next->next->data, "b"));
	assert(!strcmp(list->head->next->next->prev->data, "b"));

	assert(!strcmp(list->head->next->next->next->data, "b"));
	assert(!strcmp(list->head->next->next->next->prev->data, "b"));

	assert(list->head->next->next->next->next == NULL);

	list_destroy(list);
}

static void
_test_pop(Pool *pool)
{
	List *list = list_new(str_compare, free, pool);

	list_append(list, strdup("a"));
	list_append(list, strdup("b"));
	ListItem *tail = list_append(list, strdup("c"));

	assert(list->count == 3);

	char *a = list_pop(list);

	assert(list->count == 2);
	assert(list->tail == tail);

	char *b = list_pop(list);

	assert(list->count == 1);
	assert(list->tail == tail);

	char *c = list_pop(list);

	assert(list->count == 0);
	assert(list->tail == NULL);

	char *d = list_pop(list);

	assert(list->count == 0);
	assert(list->tail == NULL);

	assert(!strcmp(a, "a"));
	assert(!strcmp(b, "b"));
	assert(!strcmp(c, "c"));
	assert(d == NULL);

	free(a);
	free(b);
	free(c);

	list_destroy(list);
}

static void
_test_contains(Pool *pool)
{
	List *list = list_new(str_compare, free, pool);

	assert(!list_contains(list, "a"));
	assert(!list_contains(list, "b"));
	assert(!list_contains(list, "c"));

	list_append(list, strdup("a"));
	list_append(list, strdup("b"));

	assert(list_contains(list, "a"));
	assert(list_contains(list, "b"));
	assert(!list_contains(list, "c"));

	list_destroy(list);
}

static void
_test_find(Pool *pool)
{
	List *list = list_new(str_compare, free, pool);

	assert(list_find(list, NULL, (void *)"a") == NULL);
	assert(list_find(list, NULL, (void *)"b") == NULL);
	assert(list_find(list, NULL, (void *)"c") == NULL);

	list_append(list, strdup("a"));
	list_append(list, strdup("b"));

	ListItem *a = list_find(list, NULL, (void *)"a");
	ListItem *b = list_find(list, NULL, (void *)"b");
	ListItem *c = list_find(list, NULL, (void *)"c");

	assert(list->head == a);
	assert(!strcmp(a->data, "a"));
	assert(a->prev == NULL);
	assert(a->next == b);

	assert(list->tail == b);
	assert(!strcmp(b->data, "b"));
	assert(b->prev == a);
	assert(b->next == NULL);

	assert(c == NULL);

	list_destroy(list);
}

static void
_test_find_with_offset(Pool *pool)
{
	List *list = list_new(str_compare, free, pool);

	list_append(list, strdup("a"));
	ListItem *offset = list_append(list, strdup("b"));

	ListItem *a = list_find(list, offset, (void *)"a");

	assert(a == NULL);

	ListItem *b = list_find(list, offset, (void *)"b");

	assert(list->tail == b);
	assert(b == offset);
	assert(!strcmp(b->data, "b"));
	assert(b->prev != NULL);
	assert(b->next == NULL);

	list_destroy(list);
}

static void
_test_clear(Pool *pool)
{
	List *list = list_new(str_compare, free, pool);

	list_append(list, strdup("a"));
	list_append(list, strdup("b"));
	list_append(list, strdup("c"));

	assert(list->count == 3);

	list_clear(list);

	assert(list->count == 0);
	assert(list->head == NULL);
	assert(list->tail == NULL);

	list_destroy(list);
}

static void
_test_walk_up(Pool *pool)
{
	List *list = list_new(str_compare, free, pool);

	list_append(list, strdup("a"));
	list_append(list, strdup("b"));
	list_append(list, strdup("c"));

	static char *strings[] = { "a", "b", "c" };

	ListItem *iter = list_head(list);
	size_t i = 0;

	while(iter)
	{
		assert(i < 3);
		assert(!strcmp(list_item_get_data(iter), strings[i]));

		iter = list_item_next(iter);
		++i;
	}

	assert(i == 3);

	list_destroy(list);
}

static void
_test_walk_down(Pool *pool)
{
	List *list = list_new(str_compare, free, pool);

	list_prepend(list, strdup("a"));
	list_prepend(list, strdup("b"));
	list_prepend(list, strdup("c"));

	static char *strings[] = { "a", "b", "c" };

	ListItem *iter = list_tail(list);
	size_t i = 0;

	while(iter)
	{
		assert(i < 3);
		assert(!strcmp(list_item_get_data(iter), strings[i]));

		iter = list_item_prev(iter);
		++i;
	}

	assert(i == 3);

	list_destroy(list);
}

static void
_test_update_item(Pool *pool)
{
	List *list = list_new(str_compare, free, pool);

	list_append(list, strdup("a"));
	list_append(list, strdup("b"));
	list_append(list, strdup("c"));

	ListItem *item = list_find(list, NULL, "b");

	assert(item != NULL);
	assert(!strcmp(item->data, "b"));

	list_item_free_data(list, item);
	list_item_set_data(item, strdup("z"));

	assert(!strcmp(list->head->data, "a"));
	assert(!strcmp(list->head->next->data, "z"));
	assert(!strcmp(list->head->next->next->data, "c"));
	assert(list->head->next->next->next == NULL);

	list_destroy(list);
}

static void
_run_tests(Pool *pool)
{
	_test_empty(pool);
	_test_count(pool);
	_test_head(pool);
	_test_tail(pool);
	_test_prepend(pool);
	_test_append(pool);
	_test_sorted(pool);
	_test_remove_head(pool);
	_test_remove_middle(pool);
	_test_remove_tail(pool);
	_test_remove_head_by_data(pool);
	_test_remove_middle_by_data(pool);
	_test_remove_tail_by_data(pool);
	_test_remove_all_data_with_head(pool);
	_test_remove_all_data_in_middle(pool);
	_test_remove_all_data_with_tail(pool);
	_test_pop(pool);
	_test_contains(pool);
	_test_find(pool);
	_test_find_with_offset(pool);
	_test_clear(pool);
	_test_walk_up(pool);
	_test_walk_down(pool);
	_test_update_item(pool);
}

int
main(int argc, char *argv[])
{
	_test_new();

	_run_tests(NULL);

	MemoryPool *pool = memory_pool_new(sizeof(ListItem), 8);

	_run_tests((Pool *)pool);

	memory_pool_destroy(pool);
}

