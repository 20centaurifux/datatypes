# datatypes

## Introduction

**datatypes** provides various abstract data types you might miss in C.
The following types are available:

* hash table (with fixed but user-definable bucket size)
* red-black tree
* associated array
* singly-linked list
* doubly-linked list
* queue
* stack
* byte-buffer
* asynchronous (message) queue

All data types are generic. Compare and memory allocation functions are fully
customizable.

## Building

To build the library *GNU Make* is required. Please edit the Makefile to
change file locations and dependencies.

The asynchronous message queue implementation requires POSIX threads. You can
disable this feature in the related section of the Makefile.

## Documentation

To generate the code documentation please run Doxygen.
