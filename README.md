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

To build the library [GNU Make](https://www.gnu.org/software/make/) is required.
Please edit the Makefile to change file locations and dependencies.

The asynchronous message queue implementation requires POSIX threads. You can
disable this feature in the related section of the Makefile.

If you want to build the library on a Debian based distribution enter the following
commands:

```
$ sudo apt-get install build-essential git
$ git clone https://github.com/20centaurifux/datatypes.git
$ cd datatypes
```

On a x86 system open the Makefile and change the value of the *LIBDIR* setting
to

```
LIBDIR=$(PREFIX)/lib
```

Now build and install the library:

```
$ make
$ sudo make install && ldconfig
```

## Documentation

To generate the code documentation please run Doxygen.
