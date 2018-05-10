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
 * \brief A generic associative array.
 * \author Sebastian Fedrau <sebastian.fedrau@gmail.com>
 */
#define _GNU_SOURCE

#include "assocarray.h"
#include <assert.h>
#include <string.h>
#include <limits.h>

static int
_assoc_array_binary_search(CompareFunc compare, void **keys, size_t len, const void *key, ssize_t *index)
{
	ssize_t begin = 0, end = len  - 1;
	int32_t result = 0;

	if(len == 1)
	{
		*index = 0;
		return compare(key, *keys);
	}

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
	
	return result;
}

AssocArray *
assoc_array_new(CompareFunc compare_keys, FreeFunc free_key, FreeFunc free_value)
{
	AssocArray *array;

	if(!(array = (AssocArray *)malloc(sizeof(AssocArray))))
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

	if(!(array->keys = (void **)calloc(array->size, sizeof(void *))))
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		abort();
	}

	if(!(array->values = (void **)calloc(array->size, sizeof(void *))))
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

void
assoc_array_free(AssocArray *array)
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

			if(array->free_value)
			{
				array->free_value(array->values[i]);
			}
		}
	}

	free(array->keys);
	free(array->values);
}

void
assoc_array_clear(AssocArray *array)
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

			if(array->free_value)
			{
				array->free_value(array->values[i]);
			}
		}
	}

	array->count = 0;
}

void
assoc_array_set(AssocArray *array, void * restrict key, void * restrict value, bool overwrite_key)
{
	ssize_t offset = 0;

	assert(array != NULL);
	assert(key != NULL);

	if(array->count)
	{
		int result = _assoc_array_binary_search(array->compare_keys, array->keys, array->count, key, &offset);

		if(result == 0)
		{
			/* overwrite key & value */
			if(overwrite_key)
			{
				if(array->free_key)
				{
					array->free_key(array->keys[offset]);
				}

				array->keys[offset] = key;
			}

			if(array->free_value)
			{
				array->free_value(array->values[offset]);
			}

			array->values[offset] = value;
		}
		else
		{
			/* resize array if necessary */
			if(array->count == array->size)
			{
				array->size *= 2;

				if(array->size <= array->count)
				{
					fprintf(stderr, "Integer overflow.\n");
					abort();
				}

				if(array->size > SSIZE_MAX)
				{
					fprintf(stderr, "Array exceeds maximum size.\n");
					abort();
				}

				if(!(array->keys = (void **)realloc(array->keys, array->size * (sizeof(void *)))))
				{
					fprintf(stderr, "Couldn't allocate memory.\n");
					abort();
				}

				if(!(array->values = (void **)realloc(array->values, array->size * sizeof(void *))))
				{
					fprintf(stderr, "Couldn't allocate memory.\n");
					abort();
				}
			}

			if(result < 0)
			{
				/* insert key & value before offset */
				memmove(&array->keys[offset + 1], &array->keys[offset], (array->count - offset) * sizeof(void *));
				memmove(&array->values[offset + 1], &array->values[offset], (array->count - offset) * sizeof(void *));

				array->keys[offset] = key;
				array->values[offset] = value;
			}
			else
			{
				/* insert key & value after offset */
				++offset;

				if((size_t)offset < array->count)
				{
					memmove(&array->keys[offset + 1], &array->keys[offset], (array->count - offset) * sizeof(void *));
					memmove(&array->values[offset + 1], &array->values[offset], (array->count - offset) * sizeof(void *));
				}

				array->keys[offset] = key;
				array->values[offset] = value;
			}

			++array->count;
		}
	}
	else
	{
		*array->keys = key;
		*array->values = value;
		++array->count;
	}
}

void
assoc_array_remove(AssocArray *array, const void *key)
{
	ssize_t offset = 0;

	assert(array != NULL);
	assert(key != NULL);

	if(array->count)
	{
		if(!_assoc_array_binary_search(array->compare_keys, array->keys, array->count, key, &offset))
		{
			if(array->free_key)
			{
				array->free_key(array->keys[offset]);
			}

			if(array->free_value)
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

void *
assoc_array_lookup(const AssocArray *array, const void *key)
{
	ssize_t offset = 0;

	assert(array != NULL);
	assert(key != NULL);

	if(array->count)
	{
		if(!_assoc_array_binary_search(array->compare_keys, array->keys, array->count, key, &offset))
		{
			return array->values[offset];
		}
	}

	return NULL;
}

bool
assoc_array_key_exists(const AssocArray *array, const void *key)
{
	ssize_t offset;

	assert(array != NULL);
	assert(key != NULL);

	if(array->count)
	{
		return _assoc_array_binary_search(array->compare_keys, array->keys, array->count, key, &offset) == 0;
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

