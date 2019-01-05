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
 * \file datatypes.h
 * \brief General declarations.
 * \author Sebastian Fedrau <sebastian.fedrau@gmail.com>
 */
#ifndef DATATYPES_H
#define DATATYPES_H

/*! Major version of libdatatypes. */
#define DATATYPES_VERSION_MAJOR      0
/*! Minor version of libdatatypes. */
#define DATATYPES_VERSION_MINOR      3
/*! Patchlevel of libdatatypes. */
#define DATATYPES_VERSION_PATCHLEVEL 2

/*! A function to free memory. */
typedef void (*FreeFunc)(void *p);

#include "compare.h"
#include "pool.h"
#include "hash.h"
#include "hashtable.h"
#include "rbtree.h"
#include "list.h"
#include "slist.h"
#include "queue.h"
#include "stack.h"
#include "buffer.h"
#include "asyncqueue.h"
#include "assocarray.h"

#endif

