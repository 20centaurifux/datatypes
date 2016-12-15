/***************************************************************************
    begin........: May 2012
    copyright....: Sebastian Fedrau
    email........: sebastian.fedrau@gmail.com
 ***************************************************************************/

/***************************************************************************
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License v3 as published by
    the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License v3 for more details.
 ***************************************************************************/
/*!
 * \file rbtree.h
 * \brief A generic balanced binary tree.
 * \author Sebastian Fedrau <sebastian.fedrau@gmail.com>
 * \version 0.1.0
 * \date 26. June 2012
 */

#ifndef __RBTREE_H__
#define __RBTREE_H__

#include <stdbool.h>
#include <stdint.h>

#include "datatypes.h"
#include "allocator.h"

/**
 *\struct RBNode
 *\brief Structure holding node data.
 */
typedef struct _rbnode
{
	/*! Key of the node. */
	void *key;
	/*! Value of the node. */
	void *value;
	/*! Left node or NULL. */
	struct _rbnode *left;
	/*! Right node or NULL. */
	struct _rbnode *right;
	/*! true if color is black. */
	bool black;
} RBNode;

/**
 *\struct RBTree
 *\brief A red-black tree.
 */
typedef struct
{
	/*! Function to compare two keys. */
	CompareFunc compare_keys;
	/*! Function to free a key. */
	FreeFunc free_key;
	/*! Function to free a value. */
	FreeFunc free_value;
	/*! The root node. */
	RBNode *root;
	/*! Stack used when traversing tree. */
	RBNode **stack;
	/*! The stack pointer. */
	RBNode **sp;
	/*! Size of the stack. */
	size_t stack_size;
	/*! An optional allocator to create nodes. */
	Allocator *allocator;
	/*! Number of stored items. */
	size_t count;
} RBTree;

/**
 *\enum RBTreeInsertResult
 *\brief result of rbtree_set() method
 */
typedef enum
{
	/*! Node has been inserted. */
	RBTREE_INSERT_RESULT_NEW,
	/*! Node has been replaced. */
	RBTREE_INSERT_RESULT_REPLACED
} RBTreeInsertResult;

/**
 *\struct RBTreeIterStackItem
 *\brief Structure holding a node and an interation state.
 */
typedef struct
{
	/*! A node. */
	RBNode *node;
	/*! An interation state. */
	int state;
} RBTreeIterStackItem;

/**
 *\struct RBTreeIter
 *\brief A structure to iterate over the elements of a RBTree.
 */
typedef struct
{
	/*! Pointer to related tree. */
	const RBTree *tree;
	/*! A stack holding nodes and an iteration state. */
	RBTreeIterStackItem *stack;
	/*! The stack pointer. */
	RBTreeIterStackItem *sp;
	/*! Size of the stack. */
	size_t stack_size;
	/*! true if iteration has been finished. */
	bool finished;
} RBTreeIter;

/**
 *\param compare_keys function to compare two keys
 *\param free_key function to free a key
 *\param free_value function to free a value
 *\param allocator an optional memory allocator for creating/destroying RBNodes or NULL
 *\return a new RBTree
 *
 * Creates a new RBTree.
 */
RBTree *rbtree_new(CompareFunc compare_keys, FreeFunc free_key, FreeFunc free_value, Allocator *allocator);

/**
 *\param tree an uninitialized RBTree
 *\param compare_keys function to compare two keys
 *\param free_key function to free a key
 *\param free_value function to free a value
 *\param allocator an optional memory allocator for creating/destroying RBNodes or NULL
 *
 * Initializes a RBTree.
 */
void rbtree_init(RBTree *tree, CompareFunc compare_keys, FreeFunc free_key, FreeFunc free_value, Allocator *allocator);

/**
 *\param tree a RBTree
 *
 * Frees resources of the tree.
 */
void rbtree_free(RBTree *tree);

/**
 *\param tree a RBTree
 *
 * Frees resources and memory allocated for tree.
 */
void rbtree_destroy(RBTree *tree);

/**
 *\param tree a RBTree
 *
 * Removes all nodes.
 */
void rbtree_clear(RBTree *tree);

/**
 *\param tree a RBTree
 *\param key key of the item
 *\param value value of the item
 *\param replace_key true to overwrite exisiting key
 *\return RBTREE_INSERT_RESULT_NEW or RBTREE_INSERT_RESULT_REPLACED
 *
 * Inserts a new key and value in the RBTree. If replace_key has been set an exisiting key will be
 * freed using the specified free_key function and then get replaced.
 */
RBTreeInsertResult rbtree_set(RBTree *tree, void * restrict key, void * restrict value, bool replace_key);

/**
 *\param tree a RBTree
 *\return number of stored nodes
 *
 * Gets the number of stored nodes.
 */
size_t rbtree_count(const RBTree *tree);

/**
 *\param tree a RBTree
 *\param key key to find
 *\return pointer to found value or NULL
 *
 * Looks up a key in the RBTree and returns its value.
 */
void *rbtree_lookup(const RBTree *tree, const void *key);

/**
 *\param tree a RBTree
 *\param key a key
 *\return true if given key does exist
 *
 * Checks if a key does exist.
 */
bool rbtree_key_exists(const RBTree *tree, const void *key);

/**
 *\param tree a RBTree
 *\param key a key
 *\return true if node has been removed
 *
 * Removes an element from the RBTree.
 */
bool rbtree_remove(RBTree *tree, const void *key);

/**
 *\param tree a RBTree
 *\param iter an uninitialized RBTreeIter
 *
 * Initializes a key/value pair iterator and associates it with the tree. Modifying the tree while
 * using the iterator might lead to undefined behaviour.
 */
void rbtree_iter_init(const RBTree *tree, RBTreeIter *iter);

/**
 *\param iter a RBTreeIter
 *
 * Frees the iterator.
 */
void rbtree_iter_free(RBTreeIter *iter);

/**
 *\param tree a RBTree
 *\param iter an already initialized RBTreeIter
 *
 * Reuses an iterator.
 */
void rbtree_iter_reuse(const RBTree *tree, RBTreeIter *iter);

/**
 *\param iter a RBTreeIter
 *\return false if end of the RBTree has been reached
 *
 * Goes to next element of the RBTree.
 */
bool rbtree_iter_next(RBTreeIter *iter);

/**
 *\param iter a RBTreeIter
 *\return key of current element
 *
 * Retrieves the key of the current element.
 */
void *rbtree_iter_get_key(const RBTreeIter *iter);

/**
 *\param iter a RBTreeIter
 *\return value of current element
 *
 * Retrieves the value of the current element.
 */
void *rbtree_iter_get_value(const RBTreeIter *iter);

#endif

