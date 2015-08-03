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
 * \version 0.1.0
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
/*! @endcond */

Buffer *
buffer_new(size_t max_size)
{
	Buffer *buf;

	if(!(buf = (Buffer *)malloc(sizeof(Buffer))))
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
	memset(buf, 0, sizeof(Buffer));

	assert(max_size > 0);

	buf->max_size = max_size;
	buf->msize = 64;
	buf->valid = true;

	if(!(buf->data = (char *)malloc(buf->msize)))
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
	/* new buffer size should be a power of 2 - I know there are a faster solutions but
	   this very simple one should work well on different platforms and is easy to
	   understand :) */
	do
	{
		assert(from < SIZE_MAX / 2);
		from *= 2;
	} while(from < to);

	return from;
}

bool
buffer_fill(Buffer *buf, const char *data, size_t len)
{
	RETURN_VAL_IF_INVALID(buf, false);

	assert(data != NULL);

	/* test maximum buffer length */
	assert(SIZE_MAX - len > buf->len);

	if(buf->len + len > buf->max_size)
	{
		fprintf(stderr, "buffer exceeds maximum size\n");
		buf->valid = false;
	}
	else
	{
		/* test if string exceeds allocated memory block */
		assert(SIZE_MAX - len > buf->msize);

		if(len > buf->msize - buf->len)
		{
			/* resize buffer */
			size_t new_size = _buffer_new_realloc_size(buf->msize, buf->msize + len);

			if(new_size < buf->msize)
			{
				fprintf(stderr, "overflow in buf->msize calculation\n");
				buf->valid = false;

				return false;
			}

			buf->msize = new_size;

			if(!(buf->data = (char *)realloc(buf->data, buf->msize)))
			{
				fprintf(stderr, "Couldn't resize buffer.\n");
				abort();
			}
		}

		/* copy data to buffer */
		if(buf->valid)
		{
			memcpy(buf->data + buf->len, data, len);
			buf->len += len;
		}
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
			bytes = 0;
		}
	}

	return bytes;
}

static void
_buffer_copy_to(Buffer *buf, size_t count, char **dst, size_t *len)
{
	assert(buf != NULL);
	assert(dst != NULL);
	assert(len != NULL);

	if(count + 1 > *len)
	{
		*len = count;

		if(!(*dst = (char *)realloc(*dst, count + 1)))
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

	/* find line-break */
	if((ptr = memchr(buf->data, '\n', buf->len)))
	{
		size_t slen = ptr - buf->data;

		/* copy found line to destination */
		_buffer_copy_to(buf, slen, dst, len);

		/* update buffer length & content */
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

	_buffer_copy_to(buf, buf->len, dst, len);

	return true;
}

char *
buffer_to_string(Buffer *buf)
{
	RETURN_VAL_IF_INVALID(buf, NULL);

	if(buf->len)
	{
		char *str = (char *)malloc(buf->len + 1);

		if(!str)
		{
			fprintf(stderr, "Couldn't allocate memory.\n");
			abort();
		}

		memcpy(str, buf->data, buf->len);
		str[buf->len] = '\0';

		return str;
	}

	return NULL;
}

