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
/**
 * \file assocarray.c
 * \brief Generic associative array.
 * \author Sebastian Fedrau <sebastian.fedrau@gmail.com>
 */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <limits.h>

#include "assocarray.h"

static int
_assoc_array_binary_search(CompareFunc compare, void **keys, size_t len, const void *key, ssize_t *index)
{
	int32_t result = 0;

	assert(compare != NULL);
	assert(keys != NULL);
	assert(len <= ASSOC_ARRAY_MAX_SIZE);
	assert(key != NULL);
	assert(index != NULL);

	if(len == 1)
	{
		*index = 0;
		result = compare(key, *keys);
	}
	else
	{
		ssize_t begin = 0;
		ssize_t end = len - 1;

		while(begin <= end)
		{
			*index = (begin + end) / 2;

			result = compare(key, keys[*index]);

			if(result > 0)
			{
				begin = *index + 1;
			}
			else if(result < 0)
			{
				end = *index - 1;
			}
			else
			{
				break;
			}
		}
	}
	
	return result;
}

AssocArray *
assoc_array_new(CompareFunc compare_keys, FreeFunc free_key, FreeFunc free_value)
{
	assert(compare_keys != NULL);

	AssocArray *array = (AssocArray *)malloc(sizeof(AssocArray));

	if(!array)
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		abort();
	}

	assoc_array_init(array, compare_keys, free_key, free_value);

	return array;
}

void
assoc_array_init(AssocArray *array, CompareFunc compare_keys, FreeFunc free_key, FreeFunc free_value)
{
	assert(array != NULL);
	assert(compare_keys != NULL);

	array->compare_keys = compare_keys;
	array->size = 8;
	array->count = 0;
	array->free_key = free_key;
	array->free_value = free_value;
	array->pair.array = array;
	array->pair.offset = 0;

	array->keys = (void **)calloc(array->size, sizeof(void *));

	if(!array->keys)
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		abort();
	}

	array->values = (void **)calloc(array->size, sizeof(void *));

	if(!array->values)
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		abort();
	}
}

void
assoc_array_destroy(AssocArray *array)
{
	assert(array != NULL);

	assoc_array_free(array);
	free(array);
}

static void
_assoc_array_free_memory(AssocArray *array)
{
	assert(array != NULL);

	if(array->free_key || array->free_value)
	{
		for(size_t i = 0; i < array->count; ++i)
		{
			if(array->free_key)
			{
				array->free_key(array->keys[i]);
			}

			if(array->free_value && array->values[i])
			{
				array->free_value(array->values[i]);
			}
		}
	}
}

void
assoc_array_free(AssocArray *array)
{
	assert(array != NULL);

	_assoc_array_free_memory(array);

	free(array->keys);
	free(array->values);
}

void
assoc_array_clear(AssocArray *array)
{
	assert(array != NULL);

	_assoc_array_free_memory(array);
	array->count = 0;
}

static void
_assoc_array_insert_first(AssocArray *array, void *key, void *value)
{
	assert(array != NULL);
	assert(key != NULL);

	*array->keys = key;
	*array->values = value;
	++array->count;
}

static void
_assoc_array_replace(AssocArray *array, void *key, void *value, ssize_t offset, bool overwrite_key)
{
	assert(array != NULL);
	assert(key != NULL);
	assert(offset >= 0 && array->count > (size_t)offset);

	if(overwrite_key)
	{
		if(array->free_key)
		{
			array->free_key(array->keys[offset]);
		}

		array->keys[offset] = key;
	}

	if(array->free_value && array->values[offset])
	{
		array->free_value(array->values[offset]);
	}

	array->values[offset] = value;
}

static void
_assoc_array_resize_if_necessary(AssocArray *array)
{
	assert(array != NULL);

	if(array->count == array->size)
	{
		if(array->size == ASSOC_ARRAY_MAX_SIZE)
		{
			fprintf(stderr, "Array exceeds allowed maximum size.\n");
			abort();
		}

		if(array->size > ASSOC_ARRAY_MAX_SIZE / 2)
		{
			array->size = ASSOC_ARRAY_MAX_SIZE;
		}
		else
		{
			array->size *= 2;
		}

		array->keys = (void **)realloc(array->keys, array->size * sizeof(void *));

		if(!array->keys)
		{
			fprintf(stderr, "Couldn't allocate memory.\n");
			abort();
		}

		array->values = (void **)realloc(array->values, array->size * sizeof(void *));

		if(!array->values)
		{
			fprintf(stderr, "Couldn't allocate memory.\n");
			abort();
		}
	}
}

static void
_assoc_array_insert_before_offset(AssocArray *array, void *key, void *value, ssize_t offset)
{
	assert(array != NULL);
	assert(key != NULL);
	assert(offset >= 0 && array->count > (size_t)offset);

	memmove(&array->keys[offset + 1], &array->keys[offset], (array->count - offset) * sizeof(void *));
	memmove(&array->values[offset + 1], &array->values[offset], (array->count - offset) * sizeof(void *));

	array->keys[offset] = key;
	array->values[offset] = value;
}

static void
_assoc_array_insert_after_offset(AssocArray *array, void *key, void *value, ssize_t offset)
{
	assert(array != NULL);
	assert(key != NULL);
	assert(offset >= 0 && array->count > (size_t)offset);

	++offset;

	if((size_t)offset < array->count)
	{
		memmove(&array->keys[offset + 1], &array->keys[offset], (array->count - offset) * sizeof(void *));
		memmove(&array->values[offset + 1], &array->values[offset], (array->count - offset) * sizeof(void *));
	}

	array->keys[offset] = key;
	array->values[offset] = value;
}

AssocArrayInsertResult
assoc_array_set(AssocArray *array, void *key, void *value, bool overwrite_key)
{
	AssocArrayInsertResult result = ASSOCARRAY_INSERT_RESULT_FAILED;

	assert(array != NULL);
	assert(key != NULL);

	if(array->count)
	{
		ssize_t offset = 0;
		int cmp = _assoc_array_binary_search(array->compare_keys, array->keys, array->count, key, &offset);

		if(cmp)
		{
			result = ASSOCARRAY_INSERT_RESULT_NEW;

			if(array->count < SIZE_MAX)
			{
				_assoc_array_resize_if_necessary(array);

				if(cmp < 0)
				{
					_assoc_array_insert_before_offset(array, key, value, offset);
				}
				else
				{
					_assoc_array_insert_after_offset(array, key, value, offset);
				}

				++array->count;
			}
			else
			{
				fprintf(stderr, "%s: integer overflow.\n", __func__);
			}
		}
		else
		{
			result = ASSOCARRAY_INSERT_RESULT_REPLACED;

			_assoc_array_replace(array, key, value, offset, overwrite_key);
		}
	}
	else
	{
		result = ASSOCARRAY_INSERT_RESULT_NEW;

		_assoc_array_insert_first(array, key, value);
	}

	return result;
}

void
assoc_array_remove(AssocArray *array, const void *key)
{
	assert(array != NULL);
	assert(key != NULL);

	if(array->count)
	{
		ssize_t offset = 0;

		if(!_assoc_array_binary_search(array->compare_keys, array->keys, array->count, key, &offset))
		{
			if(array->free_key)
			{
				array->free_key(array->keys[offset]);
			}

			if(array->free_value && array->values[offset])
			{
				array->free_value(array->values[offset]);
			}

			if((size_t)offset < array->count - 1)
			{
				memmove(&array->keys[offset], &array->keys[offset + 1], (array->count - 1 - offset) * sizeof(void *));
				memmove(&array->values[offset], &array->values[offset + 1], (array->count - 1 - offset) * sizeof(void *));
			}

			--array->count;
		}
	}
}

AssocArrayPair *
assoc_array_lookup(AssocArray *array, const void *key)
{
	assert(array != NULL);
	assert(key != NULL);

	if(array->count)
	{
		ssize_t offset = 0;

		if(!_assoc_array_binary_search(array->compare_keys, array->keys, array->count, key, &offset))
		{
			array->pair.offset = offset;

			return &array->pair;
		}
	}

	return NULL;
}

void *
assoc_array_pair_get_key(const AssocArrayPair *pair)
{
	assert(pair != NULL);
	assert(pair->array != NULL);

	return pair->array->keys[pair->offset];
}

void *
assoc_array_pair_get_value(const AssocArrayPair *pair)
{
	assert(pair != NULL);
	assert(pair->array != NULL);

	return pair->array->values[pair->offset];
}

void
assoc_array_pair_set_value(AssocArrayPair *pair, void *value)
{
	assert(pair != NULL);
	assert(pair->array != NULL);

	if(pair->array->free_value && pair->array->values[pair->offset])
	{
		pair->array->free_value(pair->array->values[pair->offset]);
	}

	pair->array->values[pair->offset] = value;
}

bool
assoc_array_key_exists(const AssocArray *array, const void *key)
{
	assert(array != NULL);
	assert(key != NULL);

	if(array->count)
	{
		ssize_t offset = 0;

		return !_assoc_array_binary_search(array->compare_keys, array->keys, array->count, key, &offset);
	}

	return false;
}

size_t
assoc_array_count(const AssocArray *array)
{
	assert(array != NULL);

	return array->count;
}

size_t
assoc_array_size(const AssocArray *array)
{
	assert(array != NULL);

	return array->size;
}

void
assoc_array_iter_init(const AssocArray *array, AssocArrayIter *iter)
{
	assert(array != NULL);
	assert(iter != NULL);

	iter->array = array;
	iter->offset = -1;
}

bool
assoc_array_iter_next(AssocArrayIter *iter)
{
	assert(iter != NULL);

	++iter->offset;

	return (size_t)iter->offset < iter->array->count;
}

void *
assoc_array_iter_get_key(const AssocArrayIter *iter)
{
	assert(iter != NULL);

	return iter->array->keys[iter->offset];
}

void *
assoc_array_iter_get_value(const AssocArrayIter *iter)
{
	assert(iter != NULL);

	return iter->array->values[iter->offset];
}

