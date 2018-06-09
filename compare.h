/***************************************************************************
    begin........: June 2012
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
 * \file compare.h
 * \brief Compare functions.
 * \author Sebastian Fedrau <sebastian.fedrau@gmail.com>
 */
#ifndef COMPARE_H
#define COMPARE_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/*! A function to compare two values. */
typedef int32_t (*CompareFunc)(const void * restrict a, const void * restrict b);

/*! A function to check if two values are equal. */
typedef bool (*EqualFunc)(const void * restrict a, const void * restrict b);

/*! Compares two strings. Returns 0 if values are equal, or a positive integer if the first value comes after the second. */
#define str_compare (CompareFunc)strcmp

/**
 *\param a pointer to a string
 *\param b pointer to a string
 *\return true if values are equal
 *
 * Checks if two strings are equal.
 */
bool str_equal(const void * restrict a, const void * restrict b);

/**
 *\param a a pointer
 *\param b a pointer
 *\return a - b
 *
 * Compares two pointers.
 */
int32_t direct_compare(const void * restrict a, const void * restrict b);

/**
 *\param a a pointer
 *\param b a pointer
 *\return true if pointers are equal
 *
 * Tests if two pointers are equal.
 */
bool direct_equal(const void * restrict a, const void * restrict b);

#endif

