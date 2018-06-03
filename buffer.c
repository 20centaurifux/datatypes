/***************************************************************************
    begin........: April 2015
    copyright....: Sebastian Fedrau
    email........: sebastian.fedrau@gmail.com
 ***************************************************************************/

/***************************************************************************
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License v3 as published by
    the Free Software Foundation.

    This program is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License v3 for more details.
 ***************************************************************************/
/**
 * \file buffer.c
 * \brief A byte buffer.
 * \author Sebastian Fedrau <sebastian.fedrau@gmail.com>
 */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <assert.h>

#include "buffer.h"

/*! @cond INTERNAL */
#define RETURN_IF_INVALID(b) if(!buffer_is_valid(b)) return
#define RETURN_VAL_IF_INVALID(b, v) if(!buffer_is_valid(b)) return v
#define BUFFER_LIMIT (SSIZE_MAX - 1)
#define EXCEEDS_BUFFER_MAX_SIZE(buf, len) buf->max_size - buf->len < len
/*! @endcond */

Buffer *
buffer_new(size_t max_size)
{
	assert(max_size > 0 && max_size <= BUFFER_LIMIT);

	Buffer *buf = (Buffer *)malloc(sizeof(Buffer));

	if(!buf)
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		abort();
	}

	buffer_init(buf, max_size);

	return buf;
}

void
buffer_init(Buffer *buf, size_t max_size)
{
	assert(buf != NULL);
	assert(max_size > 0 && max_size <= BUFFER_LIMIT);

	memset(buf, 0, sizeof(Buffer));

	buf->max_size = max_size;
	buf->msize = 64;
	buf->valid = true;
	buf->data = (char *)malloc(buf->msize);

	if(!buf->data)
	{
		fprintf(stderr, "Couldn't allocate memory.\n");
		abort();
	}
}

void
buffer_free(Buffer *buf)
{
	assert(buf != NULL);

	free(buf->data);
}

void
buffer_destroy(Buffer *buf)
{
	assert(buf != NULL);

	buffer_free(buf);
	free(buf);
}

void
buffer_clear(Buffer *buf)
{
	assert(buf != NULL);

	buf->len = 0;
	buf->valid = true;
}

size_t
buffer_len(const Buffer *buf)
{
	assert(buf != NULL);

	RETURN_VAL_IF_INVALID(buf, 0);

	return buf->len;
}

bool
buffer_is_valid(const Buffer *buf)
{
	assert(buf != NULL);

	return buf->valid;
}

bool
buffer_is_empty(const Buffer *buf)
{
	assert(buf != NULL);

	RETURN_VAL_IF_INVALID(buf, false);

	return buf->len == 0;
}

static size_t
_buffer_new_realloc_size(size_t from, size_t to)
{
	assert(from < to);

	size_t size = from;

	while(size < to)
	{
		size *= 2;

		if(size < from) /* integer overflow */
		{
			size = to;
		}
	}

	return size;
}

bool
buffer_fill(Buffer *buf, const char *data, size_t len)
{
	assert(buf != NULL);
	assert(data != NULL);

	RETURN_VAL_IF_INVALID(buf, false);

	if(EXCEEDS_BUFFER_MAX_SIZE(buf, len))
	{
		fprintf(stderr, "Buffer exceeds allowed maximum size.\n");
		buf->valid = false;
	}
	else
	{
		if(len > buf->msize - buf->len)
		{
			size_t new_size = _buffer_new_realloc_size(buf->msize, buf->msize + len);

			buf->msize = new_size;
			buf->data = (char *)realloc(buf->data, new_size);

			if(!buf->data)
			{
				fprintf(stderr, "Couldn't resize buffer.\n");
				abort();
			}
		}

		memcpy(buf->data + buf->len, data, len);
		buf->len += len;
	}

	return buf->valid;
}

ssize_t
buffer_fill_from_fd(Buffer *buf, int fd, size_t count)
{
	assert(buf != NULL);
	assert(fd >= 0);
	assert(count <= buf->max_size);

	RETURN_VAL_IF_INVALID(buf, 0);

	ssize_t bytes;
	char data[count];

	if((bytes = read(fd, data, count)) > 0)
	{
		if(!buffer_fill(buf, data, bytes))
		{
			bytes = -1;
		}
	}

	return bytes;
}

static void
_buffer_copy_to_string(Buffer *buf, size_t count, char **dst, size_t *len)
{
	assert(buf != NULL);
	assert(dst != NULL);
	assert(len != NULL);
	assert(count <= buf->len);

	if(count + 1 > *len)
	{
		*len = count;
		*dst = (char *)realloc(*dst, count + 1);

		if(!dst)
		{
			fprintf(stderr, "Couldn't resize dst.\n");
			abort();
		}
	}

	memcpy(*dst, buf->data, count);
	(*dst)[count] = '\0';
}

bool
buffer_read_line(Buffer *buf, char **dst, size_t *len)
{
	assert(buf != NULL);
	assert(dst != NULL);
	assert(len != NULL);

	RETURN_VAL_IF_INVALID(buf, false);

	char *ptr;
	bool found = false;

	if((ptr = memchr(buf->data, '\n', buf->len)))
	{
		size_t slen = ptr - buf->data;

		_buffer_copy_to_string(buf, slen, dst, len);

		buf->len -= slen + 1;
		memmove(buf->data, ptr + 1, buf->len);

		found = true;
	}

	return found;
}

bool
buffer_flush(Buffer *buf, char **dst, size_t *len)
{
	assert(buf != NULL);
	assert(dst != NULL);
	assert(len != NULL);

	RETURN_VAL_IF_INVALID(buf, false);

	bool flushed = false;

	if(buf->len)
	{
		_buffer_copy_to_string(buf, buf->len, dst, len);
		flushed = true;
	}

	return flushed;
}

char *
buffer_to_string(const Buffer *buf)
{
	assert(buf != NULL);

	RETURN_VAL_IF_INVALID(buf, NULL);

	char *str = NULL;

	if(buf->len)
	{
		str = (char *)malloc(buf->len + 1);

		if(!str)
		{
			fprintf(stderr, "Couldn't allocate memory.\n");
			abort();
		}

		memcpy(str, buf->data, buf->len);
		str[buf->len] = '\0';
	}

	return str;
}

