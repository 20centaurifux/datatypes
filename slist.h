#ifndef __SLIST_H__
#define __SLIST_H__

#include <stdint.h>
#include <stdbool.h>

#include "datatypes.h"
#include "allocator.h"

typedef struct _SListItem
{
	void *data;
	struct _SListItem *next;
} SListItem;

typedef struct
{
	SListItem *next;
	SListItem *prev;
	SListItem *cur;
	bool finished:1;
	bool head:1;
} SListIter;

typedef struct _SList
{
	void *data;
	SListItem *head;
	SListItem *tail;
	uint32_t count;
	EqualFunc equals;
	FreeFunc free;
	Allocator *allocator;
} SList;

SList *slist_new(EqualFunc equals, FreeFunc free, Allocator *allocator);

void slist_init(SList *list, EqualFunc equals, FreeFunc free, Allocator *allocator);

void slist_free(SList *list);

void slist_destroy(SList *list);

SListItem *slist_append(SList *list, void *data);

SListItem *slist_prepend(SList *list, void *data);

uint32_t slist_count(SList *list);

bool slist_empty(SList *list);

void slist_remove(SList *list, SListIter *iter);

void slist_remove_by_data(SList *list, void *data, bool remove_all);

void *slist_pop(SList *list);

bool slist_contains(SList *list, void *data);

void slist_clear(SList *list);

bool slist_find(SList *list, void const *data, SListIter *iter);

void slist_iter_init(SList *list, SListIter *iter);

bool slist_iter_next(SListIter *iter);

void *slist_iter_get_data(SListIter *iter);

void slist_iter_set_data(SListIter *iter, void *data);

#endif

