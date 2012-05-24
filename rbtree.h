#ifndef __RBTREE_H__
#define __RBTREE_H__

#include <stdbool.h>
#include <setjmp.h>

#include "generic.h"

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
	int stack_size;
	Allocator *allocator;
	jmp_buf buf;
} RBTree;

typedef enum
{
	RBTREE_INSERT_RESULT_NEW,
	RBTREE_INSERT_RESULT_REPLACED
} RBTreeInsertResult;

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

/* lookup value */
void *rbtree_lookup(RBTree *tree, const void *key);

/* test if given key does exist in tree */
bool rbtree_key_exists(RBTree *tree, const void *key);

/* remove node from tree */
bool rbtree_remove(RBTree *tree, const void *key);

/* traverse tree */
bool rbtree_foreach(RBTree *tree, ForeachKeyValuePairFunc foreach, void *user_data);

#endif

