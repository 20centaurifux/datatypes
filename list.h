#ifndef __LIST_H__
#define __LIST_H__

#include <stdint.h>
#include <stdbool.h>

#include "datatypes.h"
#include "allocator.h"

typedef struct _ListItem
{
	void *data;
	struct _ListItem *next;
	struct _ListItem *prev;
} ListItem;

typedef struct _List
{
	void *data;
	ListItem *head;
	ListItem *tail;
	uint32_t count;
	EqualFunc equals;
	FreeFunc free;
	Allocator *allocator;
} List;

List *list_new(EqualFunc equals, FreeFunc free, Allocator *allocator);

void list_init(List *list, EqualFunc equals, FreeFunc free, Allocator *allocator);

void list_free(List *list);

void list_destroy(List *list);

ListItem *list_append(List *list, void *data);

ListItem *list_prepend(List *list, void *data);

ListItem *list_insert_sorted(List *list, void *data, CompareFunc compare);

ListItem *list_head(List *list);

ListItem *list_tail(List *list);

uint32_t list_count(List *list);

bool list_empty(List *list);

void list_remove(List *list, ListItem *item);

void list_remove_by_data(List *list, void *data, bool remove_all);

void *list_pop(List *list);

bool list_contains(List *list, void *data);

void list_clear(List *list);

ListItem *list_find(List *list, ListItem *offset, void const *data);

#define list_item_next(item) item->next

#define list_item_prev(item) item->prev

#define list_item_get_data(item) item->data

#define list_item_set_data(item, value) item->data = value

void list_reorder(List *list, ListItem *item, CompareFunc compare);

#endif

