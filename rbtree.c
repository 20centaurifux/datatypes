#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "rbtree.h"

#define RBTREE_INITIAL_BLOCK_SIZE 2

RBTree *
rbtree_new(CompareFunc compare_keys, FreeFunc free_key, FreeFunc free_value, Allocator *allocator)
{
	RBTree *tree;

	assert(compare_keys != NULL);

	if(!(tree = (RBTree *)malloc(sizeof(RBTree))))
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		exit(EXIT_FAILURE);
	}

	rbtree_init(tree, compare_keys, free_key, free_value, allocator);

	return tree;
}

void inline
rbtree_init(RBTree *tree, CompareFunc compare_keys, FreeFunc free_key, FreeFunc free_value, Allocator *allocator)
{
	if(!(tree->stack = (RBNode **)malloc(sizeof(RBNode *) * RBTREE_INITIAL_BLOCK_SIZE)))
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		exit(EXIT_FAILURE);
	}

	tree->compare_keys = compare_keys;
	tree->root = NULL;
	tree->free_key = free_key;
	tree->free_value = free_value;
	tree->sp = NULL;
	tree->stack_size = RBTREE_INITIAL_BLOCK_SIZE;
	tree->allocator = allocator;
}

static void
_rbtree_destroy_node(RBTree *tree, RBNode *node)
{
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

	if(tree->free_value)
	{
		tree->free_value(node->value);
	}

	if(tree->allocator)
	{
		tree->allocator->free(tree->allocator, node);
	}
	else
	{
		free(node);
	}
}

void
rbtree_destroy(RBTree *tree)
{
	rbtree_free(tree);
	free(tree);
}

void inline
rbtree_free(RBTree *tree)
{
	if(tree->root)
	{
		_rbtree_destroy_node(tree, tree->root);
	}

	free(tree->stack);
}

/*
 *	stack helpers:
 */
static inline bool
_rbtree_stack_push(RBTree *tree, RBNode *node)
{
	int sp;

	if(!tree->sp)
	{
		tree->sp = tree->stack;
	}
	else
	{
		if((sp = tree->sp - tree->stack) >= tree->stack_size - 1)
		{
			/* resize stack size */
			tree->stack_size *= 2;

			if(!(tree->stack = (RBNode **)realloc(tree->stack, sizeof(RBNode *) * tree->stack_size)))
			{
				fprintf(stderr, "Couldn't allocate memory.\n");
				exit(EXIT_FAILURE);
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
#define _rbnode_is_black(n)    (n == NULL ? 1 : n->black)

static inline RBNode *
_rbnode_create_new(Allocator *allocator, void *key, void *value, int black, RBNode *left, RBNode *right)
{
	RBNode *node;

	if(allocator)
	{
		node = (RBNode *)allocator->alloc(allocator);
	}
	else if(!(node = (RBNode *)malloc(sizeof(RBNode))))
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		exit(EXIT_FAILURE);
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
static inline void
_rbnode_replace(RBTree *tree, RBNode *old_node, RBNode *parent, RBNode *new_node)
{
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

static inline void
_rbnode_rotate_left(RBTree *tree, RBNode *node, RBNode *parent)
{
	RBNode *right;

	right = node->right;
	_rbnode_replace(tree, node, parent, right);
	node->right = right->left;

	right->left = node;
}

static inline void
_rbnode_rotate_right(RBTree *tree, RBNode *node, RBNode *parent)
{
	RBNode *left;

	left = node->left;
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
	RBNode *node;
	RBNode *parent;
	RBNode *grandparent;
	RBNode *great_grandparent = NULL;
	RBNode *uncle;

	node = *tree->sp;
	parent = *(tree->sp - 1);

	/* case 2 */
	if(_rbnode_is_black(parent))
	{
		return;
	}

	/* case 3 */
	grandparent = *(tree->sp - 2);
	uncle = (parent == grandparent->left) ? grandparent->right : grandparent->left;

	/* test if uncle is red */
	if(!_rbnode_is_black(uncle))
	{
		/* paint parent & uncle black */
		parent->black = 1;
		uncle->black = 1;

		/* paint grandparent red & validate case 1 */
		grandparent->black = 0;
		tree->sp -= 2;
		_rbtree_insert_case1(tree);
	}
	else
	{
		/* case 4 */
		if(node == parent->right && parent == grandparent->left)
		{
			/* rotate left about parent */
			_rbnode_rotate_left(tree, parent, grandparent);

			/* update stack */
			*(tree->sp - 1) = node;
			parent = node;
			node = node->left;
			*tree->sp = node;
		}
		else if(node == parent->left && parent == grandparent->right)
		{
			/* rotate right about parent */
			_rbnode_rotate_right(tree, parent, grandparent);

			/* update stack */
			*(tree->sp - 1) = node;
			parent = node;
			node = node->right;
			*tree->sp = node;
		}

		/* case 5 */
		if(tree->sp >= tree->stack + 3)
		{
			great_grandparent = *(tree->sp - 3);
		}

		/* repaint nodes */
		grandparent->black = 0;
		parent->black = 1;


		if(node == parent->left && parent == grandparent->left)
		{
			/* rotate right about grandparent */
			_rbnode_rotate_right(tree, grandparent, great_grandparent);
		}
		else
		{
			/* rotate left about grandparent */
			_rbnode_rotate_left(tree, grandparent, great_grandparent);
		}

		/* update stack */
		*(tree->sp - 2) = *(tree->sp - 1);
		*(tree->sp - 1) = *(tree->sp);
		--tree->sp;
	}
}

static void
_rbtree_insert_case1(RBTree *tree)
{
	RBNode *node;

	node = *tree->sp;

	/* check if curent node is root node */
	if(tree->sp == tree->stack)
	{
		/* paint root node black */
		node->black = 1;
	}
	else
	{
		_rbtree_insert_case2_to_6(tree);
	}
}

RBTreeInsertResult
rbtree_set(RBTree *tree, void * restrict key, void * restrict value, bool overwrite_key)
{
	RBNode *node;
	RBNode *new_node = NULL;
	int32_t result;

	assert(tree != NULL);
	assert(key != NULL);

	tree->sp = NULL;

	/* check if tree is empty */
	if(!tree->root)
	{
		/* insert root node */
		tree->root = _rbnode_create_new(tree->allocator, key, value, 1, NULL, NULL);
		return RBTREE_INSERT_RESULT_NEW;
	}

	node = tree->root;

	while(1)
	{
		/* push current node on stack */
		if(!_rbtree_stack_push(tree, node))
		{
			break;
		}

		/* compare keys */
		if(!(result = tree->compare_keys(key, node->key)))
		{
			/* equal keys => replace item */
			if(overwrite_key)
			{
				if(tree->free_key)
				{
					tree->free_key(node->key);
				}

				node->key = key;
			}

			if(tree->free_value)
			{
				tree->free_value(node->value);
			}

			node->value = value;

			return RBTREE_INSERT_RESULT_REPLACED;
		}
		/* search position to insert new node */
		else if(result < 0)
		{
			if(node->left)
			{
				node = node->left;
			}
			else
			{
				new_node = _rbnode_create_new(tree->allocator, key, value, 0, NULL, NULL);
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
				new_node = _rbnode_create_new(tree->allocator, key, value, 0, NULL, NULL);
				node->right = new_node;
				break;
			}
		}
	}

	if(new_node)
	{
		/* push new node on stack */
		if(_rbtree_stack_push(tree, new_node))
		{
			_rbtree_insert_case2_to_6(tree);
		}
	}

	return RBTREE_INSERT_RESULT_NEW;
}

/*
 *	search nodes:
 */
static inline RBNode *
_rbtree_find_node(RBTree *tree, const void *key, bool build_stack)
{
	RBNode *node;
	int32_t result;

	assert(tree->compare_keys != NULL);

	node = tree->root;

	while(node)
	{
		if(build_stack)
		{
			_rbtree_stack_push(tree, node);
		}

		if(!(result = tree->compare_keys(key, node->key)))
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

void *
rbtree_lookup(RBTree *tree, const void *key)
{
	const RBNode *node;

	assert(tree != NULL);
	assert(key != NULL);

	if((node = _rbtree_find_node(tree, key, false)))
	{
		return node->value;
	}

	return NULL;
}

bool
rbtree_key_exists(RBTree *tree, const void *key)
{
	return _rbtree_find_node(tree, key, false) ? true : false;
}

/*
 *	remove nodes:
 */
static inline RBNode *
_rbtree_find_max_node(RBTree *tree, RBNode *node)
{
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
	RBNode *node;
	RBNode *parent;
	RBNode *sibling;
	RBNode *grandparent = NULL;

	node = *tree->sp;
	parent = *(tree->sp - 1);

	if(parent->right == node)
	{
		sibling = parent->left;
	}
	else
	{
		sibling = parent->right;
	}

	if(_rbnode_is_black(sibling) && _rbnode_is_black(sibling->left) && _rbnode_is_black(sibling->left))
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
	RBNode *node;
	RBNode *sibling;
	RBNode *parent = NULL;
	RBNode *grandparent = NULL;

	/* check if node has red sibling */
	node = *tree->sp;
	parent = *(tree->sp - 1);

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

		if(tree->sp >= tree->stack + 2)
		{
			grandparent = *(tree->sp - 2);
		}

		if(parent->left == node)
		{
			/* rotate left about parent */
			_rbnode_rotate_left(tree, parent, grandparent);
		}
		else
		{
			/* rotate right about parent */
			_rbnode_rotate_right(tree, parent, grandparent);
		}

		/* update stack */
		_rbtree_stack_push(tree, node);
		*(tree->sp - 1) = parent;
		*(tree->sp - 2) = sibling;
	}

	_rbtree_remove_case3_to_6(tree);
}

static void
_rbtree_remove_case1(RBTree *tree)
{
	if(tree->sp > tree->stack)
	{
		_rbtree_remove_case2(tree);
	}
}

bool
rbtree_remove(RBTree *tree, const void *key)
{
	RBNode *node;
	RBNode *parent = NULL;
	RBNode *max;
	RBNode *child;

	assert(tree != NULL);
	assert(key != NULL);

	tree->sp = NULL;

	if(!(node =_rbtree_find_node(tree, key, true)))
	{
		return false;
	}

	/* check if node has two children */
	if(node->left && node->right)
	{
		/* copy key & value from predecessor */
		tree->sp = NULL;
		_rbtree_stack_push(tree, node);
		max = _rbtree_find_max_node(tree, node->left);

		if(tree->free_key)
		{
			tree->free_key(node->key);
		}

		node->key = max->key;

		if(tree->free_value)
		{
			tree->free_value(node->value);
		}

		node->value = max->value;
		node = max;
	}

	child = node->right ? node->right : node->left;

	if(tree->sp > tree->stack)
	{
		parent = *(tree->sp - 1);
	}

	if(_rbnode_is_black(node))
	{
		if(_rbnode_is_black(child))
		{
			node->black = 1;
		}
		else
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

	if(tree->allocator)
	{
		tree->allocator->free(tree->allocator, node);
	}
	else
	{
		free(node);
	}

	return true;
}

/*
 *	foreach:
 */
static void
_rbtree_foreach(jmp_buf *buf, RBNode *node, ForeachKeyValuePairFunc foreach, void *user_data)
{
	if(node->left)
	{
		_rbtree_foreach(buf, node->left, foreach, user_data);
	}

	if(node->right)
	{
		_rbtree_foreach(buf, node->right, foreach, user_data);
	}

	if(!foreach(node->key, node->value, user_data))
	{
		longjmp(*buf, 1);
	}
}

bool
rbtree_foreach(RBTree *tree, ForeachKeyValuePairFunc foreach, void *user_data)
{
	assert(tree != NULL);

	if(tree->root)
	{
		if(!setjmp(tree->buf))
		{
			_rbtree_foreach(&tree->buf, tree->root, foreach, user_data);
		}
		else
		{
			return false;
		}
	}

	return true;
}
