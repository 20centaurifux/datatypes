/***************************************************************************
    begin........: April 2012
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
 * \file buffer.h
 * \brief A byte buffer.
 * \author Sebastian Fedrau <sebastian.fedrau@gmail.com>
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#ifndef BUFFER_H
#define BUFFER_H

/**
 *\struct Buffer
 *\brief A byte buffer. If the data exceeds the maximum length
 *       the buffer becomes invalid and new data is ignored.
 */
typedef struct Buffer
{
	/*! Bytes stored in buffer. */
	char *data;
	/*! Maximum size. */
	size_t max_size;
	/*! Number of bytes stored in buffer. */
	size_t len;
	/*! Allocated memory. */
	size_t msize;
	/*! true if buffer is in an consistent state. */
	bool valid;
} Buffer;

/**
 *\param max_size maximum buffer length
 *\return a new Buffer
 *
 * Creates a new buffer.
 */
Buffer *buffer_new(size_t max_size);

/**
 *\param buf a Buffer
 *\param max_size maximum buffer length
 *
 * Initializes a buffer.
 */
void buffer_init(Buffer *buf, size_t max_size);

/**
 *\param buf Buffer to free
 *
 * Frees a buffer without freeing the buf pointer.
 */
void buffer_free(Buffer *buf);

/**
 *\param buf Buffer to free
 *
 * Frees a buffer and the buf pointer.
 */
void buffer_destroy(Buffer *buf);

/**
 *\param buf Buffer to clear
 *
 * Clears a buffer. The buffer becomes valid again.
 */
void buffer_clear(Buffer *buf);

/**
 *\param buf a Buffer
 *\return length of the buffer
 *
 * Gets the length of the buffer.
 */
size_t buffer_len(const Buffer *buf);

/**
 *\param buf a Buffer
 *\return true if the buffer is valid
 *
 * Checks if a buffer is valid.
 */
bool buffer_is_valid(const Buffer *buf);

/**
 *\param buf a Buffer
 *\return true if the buffer is empty
 *
 * Checks if a buffer is empty.
 */
bool buffer_is_empty(const Buffer *buf);

/**
 *\param buf a Buffer
 *\param data data to write to the buffer
 *\param len number of bytes to write
 *\return true on success
 *
 * Writes bytes to a buffer.
 */
bool buffer_fill(Buffer *buf, const char *data, size_t len);

/**
 *\param buf a Buffer
 *\param fd a file descriptor
 *\param count bytes to read from the file descriptor
 *\return number of bytes read from the file descriptor and written to the buffer, -1 on failure
 *
 * Reads bytes from a file and writes the data to the buffer.
 */
ssize_t buffer_fill_from_fd(Buffer *buf, int fd, size_t count);

/**
 *\param buf a Buffer
 *\param dst location to store read string
 *\param len length of dst
 *\return true if a line could be read from the buffer
 *
 * Tries to read a line from the buffer. dst will be resized automatically if necessary.
 */
bool buffer_read_line(Buffer *buf, char **dst, size_t *len);

/**
 *\param buf a Buffer
 *\param dst location to store buffer data
 *\param len buffer length
 *\return true on success
 *
 * Copies data from the buffer to a string (if buffer is valid). dst will be resized automatically if necessary.
 */
bool buffer_flush(Buffer *buf, char **dst, size_t *len);

/**
 *\param buf a Buffer
 *\return a newly-allocated string
 *
 * Converts the data from the buffer to a new string. Returns NULL if buffer is invalid.
 */
char *buffer_to_string(const Buffer *buf);

#endif

