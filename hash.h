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
 * \file hash.h
 * \brief Hash functions.
 * \author Sebastian Fedrau <sebastian.fedrau@gmail.com>
 */
#ifndef HASH_H
#define HASH_H

#include <stdint.h>

/*! Specifies the type of the hash function which is passed to hashtable_new() or hashtable_init(). */
typedef uint32_t (*HashFunc)(const void *ptr);

/**
 *\param ptr pointer to a string
 *\return hash digest
 *
 * Calculates the hash digest of a string.
 */
uint32_t str_hash(const void *ptr);

/**
 *\param ptr pointer to convert
 *\return hash digest
 *
 * Converts a pointer to a hash value.
 */
uint32_t direct_hash(const void *ptr);

#endif

