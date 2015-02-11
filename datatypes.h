/***************************************************************************
    begin........: June 2012
    copyright....: Sebastian Fedrau
    email........: lord-kefir@arcor.de
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
 * \file datatypes.h
 * \brief General declarations.
 * \author Sebastian Fedrau <lord-kefir@arcor.de>
 * \version 0.1.0
 * \date 27. June 2012
 */

#ifndef __DATATYPES_H__
#define __DATATYPES_H__

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/*! A function to compare two values. */
typedef int32_t (*CompareFunc)(const void *a, const void *b);
/*! A function to check if two values are equal. */
typedef bool (*EqualFunc)(const void *a, const void *b);
/*! A function to free memory. */
typedef void (*FreeFunc)(void *p);

 /*! Compares two strings. Return 0 if values are equal, or a positive integer if the first value comes after the second. */
#define str_compare (CompareFunc)strcmp

/**
 *\param a pointer to an integer
 *\param b pointer to an integer
 *\return 0 if values are equal, or a positive integer if the first value comes after the second
 *
 * Compares to integers.
 */
int32_t int_compare(const void *a, const void *b);

/**
 *\param a pointer to a string
 *\param b pointer to a string
 *\return true if values are equal
 *
 * Checks if two strings are equal.
 */
bool str_equal(const void * restrict a, const void * restrict b);

/**
 *\param a pointer to an integer
 *\param b pointer to an integer
 *\return true if values are equal
 *
 * Checks if two integers are equal.
 */
bool int_equal(const void * restrict a, const void * restrict b);

#endif

