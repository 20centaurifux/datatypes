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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "buffer.h"

/*! @cond INTERNAL */
#define RETURN_IF_INVALID(b) if(!buffer_is_valid(b)) return
#define RETURN_VAL_IF_INVALID(b, v) if(!buffer_is_valid(b)) return v
#define BUFFER_LIMIT (SIZE_MAX / 2)
#define EXCEEDS_BUFFER_LIMIT(buf, len) BUFFER_LIMIT - buf->len < len
#define EXCEEDS_BUFFER_MAX_SIZE(buf, len) buf->max_size - buf->len < len
/*! @endcond */

Buffer *
buffer_new(size_t max_size)
{
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
	RETURN_VAL_IF_INVALID(buf, false);

	return buf->len == 0;
}

static size_t
_buffer_new_realloc_size(size_t from, size_t to)
{
	while(from < to)
	{
		from *= 2;
		assert(from <= BUFFER_LIMIT);
	}

	return from;
}

bool
buffer_fill(Buffer *buf, const char *data, size_t len)
{
	RETURN_VAL_IF_INVALID(buf, false);

	assert(data != NULL);

	if(EXCEEDS_BUFFER_LIMIT(buf, len))
	{
		fprintf(stderr, "Buffer size exceeds supported limit.\n");
		buf->valid = false;
	}
	else if(EXCEEDS_BUFFER_MAX_SIZE(buf, len))
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
	ssize_t bytes;
	char data[count];

	RETURN_VAL_IF_INVALID(buf, 0);

	assert(fd >= 0);

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
	char *ptr;

	RETURN_VAL_IF_INVALID(buf, false);

	assert(dst != NULL);
	assert(len != NULL);

	if((ptr = memchr(buf->data, '\n', buf->len)))
	{
		size_t slen = ptr - buf->data;

		_buffer_copy_to_string(buf, slen, dst, len);

		buf->len -= slen + 1;
		memmove(buf->data, ptr + 1, buf->len);

		return true;
	}

	return false;
}

bool
buffer_flush(Buffer *buf, char **dst, size_t *len)
{
	RETURN_VAL_IF_INVALID(buf, false);

	assert(dst != NULL);
	assert(len != NULL);

	if(!buf->len)
	{
		return false;
	}

	_buffer_copy_to_string(buf, buf->len, dst, len);

	return true;
}

char *
buffer_to_string(const Buffer *buf)
{
	RETURN_VAL_IF_INVALID(buf, NULL);

	char *str = NULL;

	if(buf->len)
	{
		assert(buf->len <= BUFFER_LIMIT);

		str = (char *)malloc(buf->len + 1);

		if(!str)
		{
			fprintf(stderr, "Couldn't allocate memory.\n");
			abort();
		}

		memcpy(str, buf->data, buf->len);
		str[buf->len] = '\0';

		return str;
	}

	return str;
}

