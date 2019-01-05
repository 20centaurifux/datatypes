#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <datatypes/datatypes.h>

static void
_test_new(void)
{
	SList *slist = slist_new(str_compare, free, NULL);

	assert(slist->count == 0);
	assert(slist->head == NULL);
	assert(slist->tail == NULL);
	assert(slist->compare == str_compare);
	assert(slist->free == free);
	assert(slist->pool == NULL);

	slist_destroy(slist);
}

static void
_test_empty(Pool *pool)
{
	SList *slist = slist_new(str_compare, free, pool);

	assert(slist_empty(slist));

	slist_append(slist, strdup("a"));

	assert(!slist_empty(slist));

	slist_destroy(slist);
}

static void
_test_count(Pool *pool)
{
	SList *slist = slist_new(str_compare, free, pool);

	assert(slist->count == 0);
	assert(slist->count == slist_count(slist));

	slist_append(slist, strdup("a"));

	assert(slist->count == 1);
	assert(slist->count == slist_count(slist));

	slist_destroy(slist);
}

static void
_test_head(Pool *pool)
{
	SList *slist = slist_new(str_compare, free, pool);

	assert(slist->head == NULL);
	assert(slist->head == slist_head(slist));

	SListItem *a = slist_append(slist, strdup("a"));

	assert(slist->head == a);
	assert(slist->head == slist_head(slist));

	slist_destroy(slist);
}

static void
_test_prepend(Pool *pool)
{
	SList *slist = slist_new(str_compare, free, pool);

	SListItem *a = slist_prepend(slist, strdup("a"));

	assert(slist->count == 1);
	assert(slist->head == a);
	assert(slist->tail == a);

	SListItem *b = slist_prepend(slist, strdup("b"));

	assert(slist->count == 2);
	assert(slist->head == b);
	assert(slist->tail == a);

	assert(!strcmp(slist->head->data, "b"));
	assert(!strcmp(slist->head->next->data, "a"));
	assert(slist->head->next->next == NULL);

	slist_destroy(slist);
}

static void
_test_append(Pool *pool)
{
	SList *slist = slist_new(str_compare, free, pool);

	SListItem *a = slist_append(slist, strdup("a"));

	assert(slist->count == 1);
	assert(slist->head == a);
	assert(slist->tail == a);

	SListItem *b = slist_append(slist, strdup("b"));

	assert(slist->count == 2);
	assert(slist->head == a);
	assert(slist->tail == b);

	assert(!strcmp(slist->head->data, "a"));
	assert(!strcmp(slist->head->next->data, "b"));
	assert(slist->head->next->next == NULL);

	slist_destroy(slist);
}

static void
_test_sorted(Pool *pool)
{
	SList *slist = slist_new(str_compare, free, pool);

	SListItem *c = slist_insert_sorted(slist, strdup("c"));

	assert(slist->count == 1);
	assert(slist->head == c);
	assert(slist->tail == c);

	SListItem *b = slist_insert_sorted(slist, strdup("b"));

	assert(slist->count == 2);
	assert(slist->head == b);
	assert(slist->tail == c);

	SListItem *a = slist_insert_sorted(slist, strdup("a"));

	assert(slist->count == 3);
	assert(slist->head == a);
	assert(slist->tail == c);

	assert(!strcmp(slist->head->data, "a"));
	assert(!strcmp(slist->head->next->data, "b"));
	assert(!strcmp(slist->head->next->next->data, "c"));
	assert(slist->head->next->next->next == NULL);

	slist_destroy(slist);
}

static void
_test_remove_head(Pool *pool)
{
	SList *slist = slist_new(str_compare, free, pool);

	SListItem *a = slist_append(slist, strdup("a"));
	SListItem *b = slist_append(slist, strdup("b"));
	SListItem *c = slist_append(slist, strdup("c"));

	slist_remove(slist, a);

	assert(slist->count == 2);
	assert(slist->head == b);
	assert(slist->tail == c);

	assert(!strcmp(slist->head->data, "b"));
	assert(!strcmp(slist->head->next->data, "c"));
	assert(slist->head->next->next == NULL);

	slist_destroy(slist);
}

static void
_test_remove_middle(Pool *pool)
{
	SList *slist = slist_new(str_compare, free, pool);

	SListItem *a = slist_append(slist, strdup("a"));
	SListItem *b = slist_append(slist, strdup("b"));
	SListItem *c = slist_append(slist, strdup("c"));

	slist_remove(slist, b);

	assert(slist->count == 2);
	assert(slist->head == a);
	assert(slist->tail == c);

	assert(!strcmp(slist->head->data, "a"));
	assert(!strcmp(slist->head->next->data, "c"));
	assert(slist->head->next->next == NULL);

	slist_destroy(slist);
}

static void
_test_remove_tail(Pool *pool)
{
	SList *slist = slist_new(str_compare, free, pool);

	SListItem *a = slist_append(slist, strdup("a"));
	SListItem *b = slist_append(slist, strdup("b"));
	SListItem *c = slist_append(slist, strdup("c"));

	slist_remove(slist, c);

	assert(slist->count == 2);
	assert(slist->head == a);
	assert(slist->tail == b);

	assert(!strcmp(slist->head->data, "a"));
	assert(!strcmp(slist->head->next->data, "b"));
	assert(slist->head->next->next == NULL);

	slist_destroy(slist);
}

static void
_test_remove_head_by_data(Pool *pool)
{
	SList *slist = slist_new(str_compare, free, pool);

	slist_append(slist, strdup("a"));
	SListItem *b = slist_append(slist, strdup("b"));
	SListItem *c = slist_append(slist, strdup("c"));

	slist_remove_by_data(slist, "a", false);

	assert(slist->count == 2);
	assert(slist->head == b);
	assert(slist->tail == c);

	assert(!strcmp(slist->head->data, "b"));
	assert(!strcmp(slist->head->next->data, "c"));
	assert(slist->head->next->next == NULL);

	slist_destroy(slist);
}

static void
_test_remove_middle_by_data(Pool *pool)
{
	SList *slist = slist_new(str_compare, free, pool);

	SListItem *a = slist_append(slist, strdup("a"));
	slist_append(slist, strdup("b"));
	SListItem *c = slist_append(slist, strdup("c"));

	slist_remove_by_data(slist, "b", false);

	assert(slist->count == 2);
	assert(slist->head == a);
	assert(slist->tail == c);

	assert(!strcmp(slist->head->data, "a"));
	assert(!strcmp(slist->head->next->data, "c"));
	assert(slist->head->next->next == NULL);

	slist_destroy(slist);
}

static void
_test_remove_tail_by_data(Pool *pool)
{
	SList *slist = slist_new(str_compare, free, pool);

	SListItem *a = slist_append(slist, strdup("a"));
	SListItem *b = slist_append(slist, strdup("b"));
	slist_append(slist, strdup("c"));

	slist_remove_by_data(slist, "c", false);

	assert(slist->count == 2);
	assert(slist->head == a);
	assert(slist->tail == b);

	assert(!strcmp(slist->head->data, "a"));
	assert(!strcmp(slist->head->next->data, "b"));
	assert(slist->head->next->next == NULL);

	slist_destroy(slist);
}

static void
_test_remove_all_data_with_head(Pool *pool)
{
	SList *slist = slist_new(str_compare, free, pool);

	slist_append(slist, strdup("a"));
	slist_append(slist, strdup("b"));
	slist_append(slist, strdup("b"));
	slist_append(slist, strdup("b"));
	slist_append(slist, strdup("a"));
	slist_append(slist, strdup("c"));

	slist_remove_by_data(slist, "a", true);

	assert(slist->count == 4);

	assert(!strcmp(slist->head->data, "b"));
	assert(!strcmp(slist->head->next->data, "b"));
	assert(!strcmp(slist->head->next->next->data, "b"));
	assert(!strcmp(slist->head->next->next->next->data, "c"));
	assert(slist->head->next->next->next->next == NULL);

	slist_destroy(slist);
}

static void
_test_remove_all_data_in_middle(Pool *pool)
{
	SList *slist = slist_new(str_compare, free, pool);

	slist_append(slist, strdup("a"));
	slist_append(slist, strdup("b"));
	slist_append(slist, strdup("b"));
	slist_append(slist, strdup("b"));
	slist_append(slist, strdup("c"));

	slist_remove_by_data(slist, "b", true);

	assert(slist->count == 2);

	assert(!strcmp(slist->head->data, "a"));
	assert(!strcmp(slist->head->next->data, "c"));
	assert(slist->head->next->next == NULL);

	slist_destroy(slist);
}

static void
_test_remove_all_data_with_tail(Pool *pool)
{
	SList *slist = slist_new(str_compare, free, pool);

	slist_append(slist, strdup("c"));
	slist_append(slist, strdup("b"));
	slist_append(slist, strdup("b"));
	slist_append(slist, strdup("b"));
	slist_append(slist, strdup("a"));
	slist_append(slist, strdup("a"));

	slist_remove_by_data(slist, "a", true);

	assert(slist->count == 4);

	assert(!strcmp(slist->head->data, "c"));
	assert(!strcmp(slist->head->next->data, "b"));
	assert(!strcmp(slist->head->next->next->data, "b"));
	assert(!strcmp(slist->head->next->next->next->data, "b"));
	assert(slist->head->next->next->next->next == NULL);

	slist_destroy(slist);
}

static void
_test_pop(Pool *pool)
{
	SList *slist = slist_new(str_compare, free, pool);

	slist_append(slist, strdup("a"));
	slist_append(slist, strdup("b"));
	SListItem *tail = slist_append(slist, strdup("c"));

	assert(slist->count == 3);

	char *a = slist_pop(slist);

	assert(slist->count == 2);
	assert(slist->tail == tail);

	char *b = slist_pop(slist);

	assert(slist->count == 1);
	assert(slist->tail == tail);

	char *c = slist_pop(slist);

	assert(slist->count == 0);
	assert(slist->tail == NULL);

	char *d = slist_pop(slist);

	assert(slist->count == 0);
	assert(slist->tail == NULL);

	assert(!strcmp(a, "a"));
	assert(!strcmp(b, "b"));
	assert(!strcmp(c, "c"));
	assert(d == NULL);

	free(a);
	free(b);
	free(c);

	slist_destroy(slist);
}

static void
_test_contains(Pool *pool)
{
	SList *slist = slist_new(str_compare, free, pool);

	assert(!slist_contains(slist, "a"));
	assert(!slist_contains(slist, "b"));
	assert(!slist_contains(slist, "c"));

	slist_append(slist, strdup("a"));
	slist_append(slist, strdup("b"));

	assert(slist_contains(slist, "a"));
	assert(slist_contains(slist, "b"));
	assert(!slist_contains(slist, "c"));

	slist_destroy(slist);
}

static void
_test_find(Pool *pool)
{
	SList *slist = slist_new(str_compare, free, pool);

	assert(slist_find(slist, NULL, (void *)"a") == NULL);
	assert(slist_find(slist, NULL, (void *)"b") == NULL);
	assert(slist_find(slist, NULL, (void *)"c") == NULL);

	slist_append(slist, strdup("a"));
	slist_append(slist, strdup("b"));

	SListItem *a = slist_find(slist, NULL, (void *)"a");
	SListItem *b = slist_find(slist, NULL, (void *)"b");
	SListItem *c = slist_find(slist, NULL, (void *)"c");

	assert(slist->head == a);
	assert(!strcmp(a->data, "a"));
	assert(slist->tail == b);
	assert(!strcmp(b->data, "b"));
	assert(c == NULL);

	slist_destroy(slist);
}

static void
_test_find_with_offset(Pool *pool)
{
	SList *slist = slist_new(str_compare, free, pool);

	slist_append(slist, strdup("a"));
	SListItem *offset = slist_append(slist, strdup("b"));

	SListItem *a = slist_find(slist, offset, (void *)"a");

	assert(a == NULL);

	SListItem *b = slist_find(slist, offset, (void *)"b");

	assert(slist->tail == b);
	assert(b == offset);
	assert(!strcmp(b->data, "b"));

	slist_destroy(slist);
}

static void
_test_clear(Pool *pool)
{
	SList *slist = slist_new(str_compare, free, pool);

	slist_append(slist, strdup("a"));
	slist_append(slist, strdup("b"));
	slist_append(slist, strdup("c"));

	assert(slist->count == 3);

	slist_clear(slist);

	assert(slist->count == 0);
	assert(slist->head == NULL);
	assert(slist->tail == NULL);

	slist_destroy(slist);
}

static void
_test_walk(Pool *pool)
{
	SList *slist = slist_new(str_compare, free, pool);

	slist_append(slist, strdup("a"));
	slist_append(slist, strdup("b"));
	slist_append(slist, strdup("c"));

	static char *strings[] = { "a", "b", "c" };

	SListItem *iter = slist_head(slist);
	size_t i = 0;

	while(iter)
	{
		assert(i < 3);
		assert(!strcmp(slist_item_get_data(iter), strings[i]));

		iter = slist_item_next(iter);
		++i;
	}

	assert(i == 3);

	slist_destroy(slist);
}

static void
_test_update_item(Pool *pool)
{
	SList *slist = slist_new(str_compare, free, pool);

	slist_append(slist, strdup("a"));
	slist_append(slist, strdup("b"));
	slist_append(slist, strdup("c"));

	SListItem *item = slist_find(slist, NULL, "b");

	assert(item != NULL);
	assert(!strcmp(item->data, "b"));

	slist_item_free_data(slist, item);
	slist_item_set_data(item, strdup("z"));

	assert(!strcmp(slist->head->data, "a"));
	assert(!strcmp(slist->head->next->data, "z"));
	assert(!strcmp(slist->head->next->next->data, "c"));
	assert(slist->head->next->next->next == NULL);

	slist_destroy(slist);
}

static void
_run_tests(Pool *pool)
{
	_test_empty(pool);
	_test_count(pool);
	_test_head(pool);
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
	_test_walk(pool);
	_test_update_item(pool);
}

int
main(int argc, char *argv[])
{
	_test_new();

	_run_tests(NULL);

	MemoryPool *pool = memory_pool_new(sizeof(SListItem), 8);

	_run_tests((Pool *)pool);

	memory_pool_destroy(pool);
}

