#ifndef __LIST_H__
#define __LIST_H__

#include <stdint.h>
#include <stdbool.h>

#include "datatypes.h"

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
} List;

List *list_new(EqualFunc equals, FreeFunc free);

void list_destroy(List *list);

void list_append(List *list, void *data);

void list_prepend(List *list, void *data);

void list_insert_sorted(List *list, void *data, CompareFunc compare);

ListItem *list_head(List *list);

ListItem *list_tail(List *list);

uint32_t list_count(List *list);

bool list_empty(List *list);

void list_remove(List *list, void *data, bool remove_all);

bool list_contains(List *list, void *data);

ListItem *list_next(ListItem *item);

ListItem *list_prev(ListItem *item);

#endif

