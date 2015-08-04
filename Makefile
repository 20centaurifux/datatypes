# destination paths:
PREFIX=/usr/local
INCDIR=$(PREFIX)/include
LIBDIR=$(PREFIX)/lib64

# compiler:
CC=gcc
CFLAGS=-Wall -std=c99 -O2 -fPIC $(OPENMP_CFLAGS) $(PTHREAD_CFLAGS)

# source code & object files:
SRC=.
SRCS=$(SRC)/allocator.c $(SRC)/asyncqueue.c $(SRC)/buffer.c $(SRC)/datatypes.c \
     $(SRC)/hashtable.c $(SRC)/list.c $(SRC)/rbtree.c $(SRC)/slist.c $(SRC)/stack.c
OBJS=$(SRCS:.c=.o)

# optional libraries (comment assigned values out to disable):
PTHREAD_CFLAGS=-DWITH_PTHREAD
OPENMP_CFLAGS=-DWITH_OPENMP -fopenmp

# version information:
MAJOR_VERSION=0
MINOR_VERSION=1
PATCHLEVEL=0

# destination library files:
STATIC_LIB=libdatatypes-$(MAJOR_VERSION).$(MINOR_VERSION).$(PATCHLEVEL).a
SHARED_LIB=$(STATIC_LIB:.a=.so)

# targets:
.PHONY: all clean install uninstall

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

all: $(OBJS)
	ar rcs $(SRC)/$(STATIC_LIB) $(OBJS)
	$(CC) -shared -Wl,-soname,libdatatypes.so.0 -o $(SRC)/$(SHARED_LIB) $(OBJS)

clean:
	rm -f $(OBJS)
	rm -f $(SRC)/$(STATIC_LIB)
	rm -f $(SRC)/$(SHARED_LIB)

install:
	test -d "$(INCDIR)/datatypes" || mkdir -p "$(INCDIR)/datatypes"
	chmod 755 "$(INCDIR)/datatypes"
	cp $(SRC)/*.h "$(INCDIR)/datatypes"
	chmod 644 $(INCDIR)/datatypes/*.h
	test -d "$(LIBDIR)" || mkdir -p "$(LIBDIR)"
	cp $(SRC)/$(STATIC_LIB) "$(LIBDIR)"
	chmod 755 $(LIBDIR)/$(STATIC_LIB)
	cp $(SRC)/$(SHARED_LIB) "$(LIBDIR)"
	chmod 755 $(LIBDIR)/$(SHARED_LIB)

uninstall:
	rm -fr "$(INCDIR)/datatypes"
	rm -f $(LIBDIR)/$(STATIC_LIB)
	rm -f $(LIBDIR)/$(SHARED_LIB)
