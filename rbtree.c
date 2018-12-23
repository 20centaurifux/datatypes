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
 * \file rbtree.c
 * \brief Generic red-black tree.
 * \author Sebastian Fedrau <sebastian.fedrau@gmail.com>
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "rbtree.h"

/*! Initial stack size. */
#define RBTREE_INITIAL_BLOCK_SIZE 4

RBTree *
rbtree_new(CompareFunc compare_keys, FreeFunc free_key, FreeFunc free_value, Pool *pool)
{
	assert(compare_keys != NULL);

	RBTree *tree = (RBTree *)malloc(sizeof(RBTree));

	if(!tree)
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		abort();
	}

	rbtree_init(tree, compare_keys, free_key, free_value, pool);

	return tree;
}

void
rbtree_init(RBTree *tree, CompareFunc compare_keys, FreeFunc free_key, FreeFunc free_value, Pool *pool)
{
	assert(tree != NULL);
	assert(compare_keys != NULL);

	memset(tree, 0, sizeof(RBTree));

	tree->stack = (RBNode **)malloc(sizeof(RBNode *) * RBTREE_INITIAL_BLOCK_SIZE);

	if(!tree->stack)
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		abort();
	}

	tree->compare_keys = compare_keys;
	tree->free_key = free_key;
	tree->free_value = free_value;
	tree->stack_size = RBTREE_INITIAL_BLOCK_SIZE;
	tree->pool = pool;
	tree->pair.free_value = free_value;
	tree->pair.node = NULL;
}

static void
_rbtree_destroy_node(RBTree *tree, RBNode *node)
{
	assert(tree != NULL);
	assert(node != NULL);

	if(node->left)
	{
		_rbtree_destroy_node(tree, node->left);
	}

	if(node->right)
	{
		_rbtree_destroy_node(tree, node->right);
	}

	if(tree->free_key)
	{
		tree->free_key(node->key);
	}

	if(tree->free_value && node->value)
	{
		tree->free_value(node->value);
	}

	if(tree->pool)
	{
		tree->pool->free(tree->pool, node);
	}
	else
	{
		free(node);
	}
}

void
rbtree_destroy(RBTree *tree)
{
	assert(tree != NULL);

	rbtree_free(tree);
	free(tree);
}

void
rbtree_free(RBTree *tree)
{
	assert(tree != NULL);

	if(tree->root)
	{
		_rbtree_destroy_node(tree, tree->root);
	}

	free(tree->stack);
}

void
rbtree_clear(RBTree *tree)
{
	assert(tree != NULL);

	if(tree->root)
	{
		_rbtree_destroy_node(tree, tree->root);
	}

	tree->sp = NULL;
	tree->count = 0;
	tree->root = NULL;
}

size_t
rbtree_count(const RBTree *tree)
{
	assert(tree != NULL);

	return tree->count;
}

/*
 *	stack helpers:
 */
static bool
_rbtree_stack_push(RBTree *tree, RBNode *node)
{
	assert(tree != NULL);
	assert(node != NULL);

	if(!tree->sp)
	{
		tree->sp = tree->stack;
	}
	else
	{
		int sp = tree->sp - tree->stack;

		if(sp >= 0 && (size_t)sp >= tree->stack_size - 1)
		{
			if(tree->stack_size > (INT32_MAX / 2))
			{
				fprintf(stderr, "%s: maximum stack size reached\n", __func__);
				abort();
			}

			tree->stack_size *= 2;
			tree->stack = (RBNode **)realloc(tree->stack, sizeof(RBNode *) * tree->stack_size);

			if(!tree->stack)
			{
				fprintf(stderr, "Couldn't allocate memory.\n");
				abort();
			}

			tree->sp = tree->stack + sp;
		}

		++tree->sp;
	}

	*tree->sp = node;

	return true;
}

/*
 *	node helpers:
 */

/*! @cond INTERNAL */
#define _rbnode_is_black(n) (n == NULL ? 1 : n->black)
/*! @endcond */

static RBNode *
_rbnode_create_new(Pool *pool, void *key, void *value, int black, RBNode *left, RBNode *right)
{
	assert(key != NULL);

	RBNode *node;

	if(pool)
	{
		node = (RBNode *)pool->alloc(pool);
	}
	else if(!(node = (RBNode *)malloc(sizeof(RBNode))))
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		abort();
	}

	node->key = key;
	node->value = value;
	node->black = black;
	node->left = left;
	node->right = right;

	return node;
}

/*
 *	replace & rotate nodes:
 */
static void
_rbnode_replace(RBTree *tree, RBNode *old_node, RBNode *parent, RBNode *new_node)
{
	assert(tree != NULL);

	if(parent)
	{
		if(old_node == parent->left)
		{
			parent->left = new_node;
		}
		else
		{
			parent->right = new_node;
		}
	}
	else
	{
		tree->root = new_node;
	}
}

static void
_rbnode_rotate_left(RBTree *tree, RBNode *node, RBNode *parent)
{
	assert(tree != NULL);
	assert(node != NULL);

	RBNode *right = node->right;

	assert(right != NULL);

	_rbnode_replace(tree, node, parent, right);
	node->right = right->left;

	right->left = node;
}

static void
_rbnode_rotate_right(RBTree *tree, RBNode *node, RBNode *parent)
{
	assert(tree != NULL);
	assert(node != NULL);

	RBNode *left = node->left;
	_rbnode_replace(tree, node, parent, left);
	node->left = left->right;

	left->right = node;
}

/*
 *	insert nodes:
 */
static void _rbtree_insert_case1(RBTree *tree);

static void
_rbtree_insert_case2_to_6(RBTree *tree)
{
	assert(tree != NULL);

	RBNode *node = *tree->sp;
	RBNode *parent = *(tree->sp - 1);

	/* case 2 */
	if(_rbnode_is_black(parent))
	{
		return;
	}

	/* case 3 */
	RBNode *grandparent = *(tree->sp - 2);
	RBNode *uncle = (parent == grandparent->left) ? grandparent->right : grandparent->left;

	if(!_rbnode_is_black(uncle))
	{
		parent->black = 1;
		uncle->black = 1;

		grandparent->black = 0;
		tree->sp -= 2;
		_rbtree_insert_case1(tree);
	}
	else
	{
		/* case 4 */
		if(node == parent->right && parent == grandparent->left)
		{
			_rbnode_rotate_left(tree, parent, grandparent);

			*(tree->sp - 1) = node;
			parent = node;
			node = node->left;
			*tree->sp = node;
		}
		else if(node == parent->left && parent == grandparent->right)
		{
			_rbnode_rotate_right(tree, parent, grandparent);

			*(tree->sp - 1) = node;
			parent = node;
			node = node->right;
			*tree->sp = node;
		}

		/* case 5 */
		RBNode *great_grandparent = NULL;

		if(tree->sp >= tree->stack + 3)
		{
			great_grandparent = *(tree->sp - 3);
		}

		grandparent->black = 0;
		parent->black = 1;

		if(node == parent->left && parent == grandparent->left)
		{
			_rbnode_rotate_right(tree, grandparent, great_grandparent);
		}
		else
		{
			_rbnode_rotate_left(tree, grandparent, great_grandparent);
		}

		*(tree->sp - 2) = *(tree->sp - 1);
		*(tree->sp - 1) = *(tree->sp);
		--tree->sp;
	}
}

static void
_rbtree_insert_case1(RBTree *tree)
{
	assert(tree != NULL);

	RBNode *node = *tree->sp;

	if(tree->sp == tree->stack)
	{
		node->black = 1;
	}
	else
	{
		_rbtree_insert_case2_to_6(tree);
	}
}

RBTreeInsertResult
rbtree_set(RBTree *tree, void *key, void *value, bool overwrite_key)
{
	assert(tree != NULL);
	assert(tree->compare_keys != NULL);
	assert(key != NULL);

	tree->sp = NULL;

	if(!tree->root)
	{
		tree->root = _rbnode_create_new(tree->pool, key, value, 1, NULL, NULL);
		++tree->count;

		return RBTREE_INSERT_RESULT_NEW;
	}

	RBNode *node = tree->root;
	RBNode *new_node = NULL;

	while(1)
	{
		if(!_rbtree_stack_push(tree, node))
		{
			break;
		}

		int32_t result = tree->compare_keys(key, node->key);

		if(!result)
		{
			if(overwrite_key)
			{
				if(tree->free_key)
				{
					tree->free_key(node->key);
				}

				node->key = key;
			}

			if(tree->free_value && node->value)
			{
				tree->free_value(node->value);
			}

			node->value = value;

			return RBTREE_INSERT_RESULT_REPLACED;
		}
		else if(result < 0)
		{
			if(node->left)
			{
				node = node->left;
			}
			else
			{
				new_node = _rbnode_create_new(tree->pool, key, value, 0, NULL, NULL);
				node->left = new_node;
				break;
			}
		}
		else if(result > 0)
		{
			if(node->right)
			{
				node = node->right;
			}
			else
			{
				new_node = _rbnode_create_new(tree->pool, key, value, 0, NULL, NULL);
				node->right = new_node;
				break;
			}
		}
	}

	if(new_node)
	{
		if(_rbtree_stack_push(tree, new_node))
		{
			_rbtree_insert_case2_to_6(tree);
		}
	}

	++tree->count;

	return RBTREE_INSERT_RESULT_NEW;
}

/*
 *	search nodes:
 */
static RBNode *
_rbtree_find_node(RBTree *tree, const void *key, bool build_stack)
{
	assert(tree != NULL);
	assert(tree->compare_keys != NULL);
	assert(key != NULL);

	RBNode *node = tree->root;

	while(node)
	{
		if(build_stack)
		{
			_rbtree_stack_push(tree, node);
		}

		int32_t result = tree->compare_keys(key, node->key);

		if(!result)
		{
			return node;
		}
		else
		{
			if(result < 0)
			{
				node = node->left;
			}
			else
			{
				node = node->right;
			}
		}
	}

	return NULL;
}

RBTreePair *
rbtree_lookup(RBTree *tree, const void *key)
{
	assert(tree != NULL);
	assert(key != NULL);

	RBNode *node = _rbtree_find_node((RBTree *)tree, key, false);

	if(node)
	{
		tree->pair.node = node;

		return &tree->pair;
	}

	return NULL;
}

void *
rbtree_pair_get_key(const RBTreePair *pair)
{
	assert(pair != NULL);
	assert(pair->node != NULL);

	if(pair)
	{
		return pair->node->key;
	}

	return NULL;
}

void *
rbtree_pair_get_value(const RBTreePair *pair)
{
	assert(pair != NULL);
	assert(pair->node != NULL);

	if(pair)
	{
		return pair->node->value;
	}

	return NULL;
}

void
rbtree_pair_set_value(RBTreePair *pair, void *value)
{
	assert(pair != NULL);
	assert(pair->node != NULL);

	if(pair->free_value && pair->node->value)
	{
		pair->free_value(pair->node->value);
	}

	pair->node->value = value;
}

bool
rbtree_key_exists(const RBTree *tree, const void *key)
{
	assert(tree != NULL);
	assert(key != NULL);

	return _rbtree_find_node((RBTree *)tree, key, false) ? true : false;
}

/*
 *	remove nodes:
 */
static RBNode *
_rbtree_find_max_node(RBTree *tree, RBNode *node)
{
	assert(tree != NULL);
	assert(node != NULL);

	_rbtree_stack_push(tree, node);

	while(node->right)
	{
		node = node->right;
		_rbtree_stack_push(tree, node);
	}

	return node;
}

static void _rbtree_remove_case1(RBTree *tree);

static void
_rbtree_remove_case3_to_6(RBTree *tree)
{
	assert(tree != NULL);

	RBNode *node = *tree->sp;
	RBNode *parent = *(tree->sp - 1);
	RBNode *sibling;

	if(parent->right == node)
	{
		sibling = parent->left;
	}
	else
	{
		sibling = parent->right;
	}

	assert(sibling != NULL);

	if(_rbnode_is_black(sibling) && _rbnode_is_black(sibling->left) && _rbnode_is_black(sibling->right))
	{
		if(_rbnode_is_black(parent))
		{
			/* case 3 */
			sibling->black = 0;
			--tree->sp;
			_rbtree_remove_case1(tree);

			return;
		}
		else
		{
			/* case 4 */
			sibling->black = 0;
			parent->black = 1;

			return;
		}
	}

	/* case 5 */
	if(node == parent->left && _rbnode_is_black(sibling) && !_rbnode_is_black(sibling->left) && _rbnode_is_black(sibling->right))
	{
		sibling->black = 0;
		sibling->left->black = 1;
		_rbnode_rotate_right(tree, sibling, parent);
		sibling = parent->right;
	}
	else if(node == parent->right && _rbnode_is_black(sibling) && _rbnode_is_black(sibling->left) && !_rbnode_is_black(sibling->right))
	{
		sibling->black = 0;
		sibling->right->black = 1;
		_rbnode_rotate_left(tree, sibling, parent);
		sibling = parent->left;
	}

	/* case 6 */
	sibling->black = parent->black;
	parent->black = 1;

	RBNode *grandparent = NULL;

	if(tree->sp >= tree->stack + 2)
	{
		grandparent = *(tree->sp - 2);
	}

	if(node == parent->left)
	{
		assert(!_rbnode_is_black(sibling->right));
		sibling->right->black = 1;
		_rbnode_rotate_left(tree, parent, grandparent);
	}
	else
	{
		assert(!_rbnode_is_black(sibling->left));
		sibling->left->black = 1;
		_rbnode_rotate_right(tree, parent, grandparent);
	}
}

static void
_rbtree_remove_case2(RBTree *tree)
{
	assert(tree != NULL);

	RBNode *node = *tree->sp;
	RBNode *parent = *(tree->sp - 1);
	RBNode *sibling;

	if(parent->right == node)
	{
		sibling = parent->left;
	}
	else
	{
		sibling = parent->right;
	}

	if(!_rbnode_is_black(sibling))
	{
		parent->black = 0;
		sibling->black = 1;

		RBNode *grandparent = NULL;

		if(tree->sp >= tree->stack + 2)
		{
			grandparent = *(tree->sp - 2);
		}

		if(parent->left == node)
		{
			_rbnode_rotate_left(tree, parent, grandparent);
		}
		else
		{
			_rbnode_rotate_right(tree, parent, grandparent);
		}

		_rbtree_stack_push(tree, node);
		*(tree->sp - 1) = parent;
		*(tree->sp - 2) = sibling;
	}

	_rbtree_remove_case3_to_6(tree);
}

static void
_rbtree_remove_case1(RBTree *tree)
{
	assert(tree != NULL);

	if(tree->sp > tree->stack)
	{
		_rbtree_remove_case2(tree);
	}
}

bool
rbtree_remove(RBTree *tree, const void *key)
{
	assert(tree != NULL);
	assert(key != NULL);

	tree->sp = NULL;

	RBNode *node =_rbtree_find_node(tree, key, true);

	if(!node)
	{
		return false;
	}

	--tree->count;

	if(tree->free_key)
	{
		tree->free_key(node->key);
	}

	if(tree->free_value && node->value)
	{
		tree->free_value(node->value);
	}

	if(node->left && node->right)
	{
		RBNode *max = _rbtree_find_max_node(tree, node->left);

		node->key = max->key;
		node->value = max->value;

		node = max;
	}

	RBNode *child = node->right ? node->right : node->left;
	RBNode *parent = NULL;

	if(tree->sp > tree->stack)
	{
		parent = *(tree->sp - 1);
	}

	if(_rbnode_is_black(node))
	{
		if(!_rbnode_is_black(child))
		{
			node->black = 0;
		}

		_rbtree_remove_case1(tree);
	}

	_rbnode_replace(tree, node, parent, child);

	if(!parent && child)
	{
		child->black = 1;
	}

	if(tree->pool)
	{
		tree->pool->free(tree->pool, node);
	}
	else
	{
		free(node);
	}

	return true;
}

/*
 *	iterator:
 */
void
rbtree_iter_init(const RBTree *tree, RBTreeIter *iter)
{
	assert(tree != NULL);
	assert(iter != NULL);

	iter->tree = tree;
	iter->stack_size = tree->count + 1;
	iter->sp = NULL;
	iter->finished = false;

	if(tree->root)
	{
		iter->stack = (RBTreeIterStackItem *)malloc(sizeof(RBTreeIterStackItem) * iter->stack_size);

		if(!iter->stack)
		{
			fprintf(stderr, "Couldn't allocate memory.\n");
			abort();
		}
	}
	else
	{
		iter->finished = true;
	}
}

void
rbtree_iter_free(RBTreeIter *iter)
{
	assert(iter != NULL);

	free(iter->stack);
}

void
rbtree_iter_reuse(const RBTree *tree, RBTreeIter *iter)
{
	assert(tree != NULL);
	assert(iter != NULL);

	iter->tree = tree;
	iter->sp = NULL;

	if(tree->root)
	{
		if(tree->count + 1 > iter->stack_size)
		{
			iter->stack_size = tree->count + 1;
			iter->stack = (RBTreeIterStackItem *)realloc(iter->stack, sizeof(RBTreeIterStackItem) * iter->stack_size);

			if(!iter->stack)
			{
				fprintf(stderr, "Couldn't allocate memory.\n");
				abort();
			}
		}

		iter->finished = false;
	}
	else
	{
		iter->finished = true;
	}
}

bool
rbtree_iter_next(RBTreeIter *iter)
{
	assert(iter != NULL);

	if(iter->finished)
	{
		return false;
	}

	if(iter->sp)
	{
		for(;;)
		{
			assert(iter->sp->state >= 0 && iter->sp->state <= 2);

			if(iter->sp->state == 0)
			{
				++iter->sp->state;
				++iter->sp;
				iter->sp->node = (iter->sp - 1)->node->left;
				iter->sp->state = 0;
			}
			else if(iter->sp->state == 1)
			{
				++iter->sp->state;
				++iter->sp;
				iter->sp->node = (iter->sp - 1)->node->right;
				iter->sp->state = 0;
			}
			else if(iter->sp->state == 2)
			{
				++iter->sp->state;
				--iter->sp;
			}

			if(iter->sp < iter->stack)
			{
				iter->finished = true;

				return false;
			}

			if(iter->sp->node)
			{
				if(iter->sp->node && iter->sp->state == 0)
				{
					return true;
				}
			}
			else
			{
				--iter->sp;
			}
		}
	}
	else
	{
		iter->sp = iter->stack;
		iter->sp->node = iter->tree->root;
		iter->sp->state = 0;
	}

	return true;
}

void *
rbtree_iter_get_key(const RBTreeIter *iter)
{
	assert(iter != NULL);

	if(iter->sp && iter->sp->node)
	{
		return iter->sp->node->key;
	}

	return NULL;
}

void *
rbtree_iter_get_value(const RBTreeIter *iter)
{
	assert(iter != NULL);

	if(iter->sp && iter->sp->node)
	{
		return iter->sp->node->value;
	}

	return NULL;
}

