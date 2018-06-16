# libdatatypes

## Introduction

**libdatatypes** provides various abstract data types in plain C (C11). It's fast,
has a small memory footprint and is well-documented. The library has been tested
on x86, x86-64 and ARMv6 with GCC and Clang.

The following data types are available:

* hash table
* red-black tree
* associated array
* singly-linked list
* doubly-linked list
* queue
* stack
* byte-buffer
* asynchronous (message) queue

All data types are generic. Compare and memory allocation functions are
customizable.

## Building

To build the library [GNU Make](https://www.gnu.org/software/make/) is required.
Please edit the Makefile to change file locations and dependencies.

The asynchronous message queue implementation requires POSIX threads. You can
disable this feature in the related section of the Makefile.

If you want to build the library on a Debian based distribution enter the following
commands:

    $ sudo apt-get install build-essential git
    $ git clone https://github.com/20centaurifux/datatypes.git
    $ cd datatypes

Now build and install the library:

    $ make
    $ sudo make install && ldconfig

## Documentation

To generate the code documentation please run Doxygen. You find example code in the
"test" directory.
