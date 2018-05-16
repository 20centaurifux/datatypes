# destination paths:
PREFIX?=/usr
INCDIR?=$(PREFIX)/include

MACHINE:=$(shell uname -m)

ifeq ($(MACHINE), x86_64)
	LIBDIR?=$(PREFIX)/lib64
else
	LIBDIR?=$(PREFIX)/lib
endif

# compiler & tools:
CC?=gcc
AR?=ar
CFLAGS=-Wall -Wextra -Wno-unused-parameter -std=c11 -O2 -DNDEBUG -fPIC $(PTHREAD_CFLAGS)
CPPCHECK?=cppcheck
DOXYGEN?=doxygen

# source code & object files:
SRC=.
SRCS=$(SRC)/pool.c $(SRC)/asyncqueue.c $(SRC)/buffer.c $(SRC)/datatypes.c \
     $(SRC)/hashtable.c $(SRC)/list.c $(SRC)/rbtree.c $(SRC)/slist.c $(SRC)/stack.c $(SRC)/assocarray.c
OBJS=$(SRCS:.c=.o)

# optional libraries (comment assigned values out to disable):
PTHREAD_CFLAGS=-DWITH_PTHREAD
PTHREAD_LIB=-pthread

LDFLAGS=$(PTHREAD_LIB)

# version information:
MAJOR_VERSION=0
MINOR_VERSION=3
PATCHLEVEL=0

# destination library files:
STATIC_LIB=libdatatypes-$(MAJOR_VERSION).$(MINOR_VERSION).$(PATCHLEVEL).a
SHARED_LIB=$(STATIC_LIB:.a=.so)
STATIC_LIB_SYMLINK=libdatatypes-$(MAJOR_VERSION).$(MINOR_VERSION).a
SHARED_LIB_SYMLINK=$(STATIC_LIB_SYMLINK:.a=.so)

# targets:
.PHONY: all clean install uninstall

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

all: $(OBJS)
	$(AR) rcs $(SRC)/$(STATIC_LIB) $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -Wl,-soname,libdatatypes.so.0 -o $(SRC)/$(SHARED_LIB) $(OBJS) $(LDFLAGS)

clean:
	rm -f $(OBJS)
	rm -f $(SRC)/$(STATIC_LIB)
	rm -f $(SRC)/$(SHARED_LIB)
	rm -fr $(SRC)/doc

install:
	test -d $(DESTDIR)$(INCDIR)/datatypes || mkdir -p $(DESTDIR)$(INCDIR)/datatypes
	chmod 755 $(DESTDIR)$(INCDIR)/datatypes
	cp $(SRC)/*.h $(DESTDIR)$(INCDIR)/datatypes
	chmod 644 $(DESTDIR)$(INCDIR)/datatypes/*.h
	test -d $(DESTDIR)$(LIBDIR) || mkdir -p $(DESTDIR)$(LIBDIR)
	cp $(SRC)/$(STATIC_LIB) $(DESTDIR)$(LIBDIR)
	chmod 755 $(DESTDIR)$(LIBDIR)/$(STATIC_LIB)
	cp $(SRC)/$(SHARED_LIB) $(DESTDIR)$(LIBDIR)
	chmod 755 $(DESTDIR)$(LIBDIR)/$(SHARED_LIB)
	ln -fs $(DESTDIR)$(LIBDIR)/$(STATIC_LIB) $(DESTDIR)$(LIBDIR)/$(STATIC_LIB_SYMLINK)
	ln -fs $(DESTDIR)$(LIBDIR)/$(SHARED_LIB) $(DESTDIR)$(LIBDIR)/$(SHARED_LIB_SYMLINK)

uninstall:
	rm -fr $(DESTDIR)$(INCDIR)/datatypes
	rm -f $(DESTDIR)$(LIBDIR)/$(STATIC_LIB)
	rm -f $(DESTDIR)$(LIBDIR)/$(SHARED_LIB)
	rm -f $(DESTDIR)$(LIBDIR)/$(STATIC_LIB_SYMLINK)
	rm -f $(DESTDIR)$(LIBDIR)/$(SHARED_LIB_SYMLINK)

cppcheck:
	$(CPPCHECK) --enable=style --enable=performance --enable=information --std=c99 --force -j2 --template gcc *.h *.c

doc:
	$(DOXYGEN) $(SRC)/doxygen_config
