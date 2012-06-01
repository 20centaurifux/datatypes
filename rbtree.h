#ifndef __RBTREE_H__
#define __RBTREE_H__

#include <stdbool.h>
#include <setjmp.h>
#include <stdint.h>

#include "datatypes.h"

typedef struct _rbnode
{
	void *key;
	void *value;
	struct _rbnode *left;
	struct _rbnode *right;
	bool black;
} RBNode;

typedef struct
{
	CompareFunc compare_keys;
	FreeFunc free_key;
	FreeFunc free_value;
	RBNode *root;
	RBNode **stack;
	RBNode **sp;
	int32_t stack_size;
	Allocator *allocator;
	jmp_buf buf;
	uint32_t count;
} RBTree;

typedef enum
{
	RBTREE_INSERT_RESULT_NEW,
	RBTREE_INSERT_RESULT_REPLACED
} RBTreeInsertResult;

typedef struct
{
	RBNode *node;
	int state;
} RBTreeIterStackItem;

typedef struct
{
	RBTree *tree;
	RBTreeIterStackItem *stack;
	RBTreeIterStackItem *sp;
	uint32_t stack_size;
	bool finished;
} RBTreeIter;

/* return pointer to a new allocated tree */
RBTree *rbtree_new(CompareFunc compare_keys, FreeFunc free_key, FreeFunc free_value, Allocator *allocator);

/* initialize an already existing tree */
void rbtree_init(RBTree *tree, CompareFunc compare_keys, FreeFunc free_key, FreeFunc free_value, Allocator *allocator);

/* free resources and memory allocated for tree */
void rbtree_destroy(RBTree *tree);

/* free resources */
void rbtree_free(RBTree *tree);

/* save key & value in tree */
RBTreeInsertResult rbtree_set(RBTree *tree, void * restrict key, void * restrict value, bool replace_key);

uint32_t rbtree_count(RBTree *tree);

/* lookup value */
void *rbtree_lookup(RBTree *tree, const void *key);

/* test if given key does exist in tree */
bool rbtree_key_exists(RBTree *tree, const void *key);

/* remove node from tree */
bool rbtree_remove(RBTree *tree, const void *key);

void rbtree_iter_init(RBTree *tree, RBTreeIter *iter);

void rbtree_iter_free(RBTreeIter *iter);

void rbtree_iter_reuse(RBTree *tree, RBTreeIter *iter);

bool rbtree_iter_next(RBTreeIter *iter);

void *rbtree_iter_get_key(RBTreeIter *iter);

void *rbtree_iter_get_value(RBTreeIter *iter);

#endif

