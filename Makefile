CC=gcc
CFLAGS = -Wall -std=c99 -O3 -DNDEBUG -DOPENMP -fopenmp

MAJOR_VERSION=0
MINOR_VERSION=1
PATCHLEVEL=0

all:
	$(CC) -c hashtable.c -o hashtable.o $(CFLAGS)
	$(CC) -c rbtree.c -o rbtree.o $(CFLAGS)
	$(CC) -c slist.c -o slist.o $(CFLAGS)
	$(CC) -c list.c -o list.o $(CFLAGS)
	$(CC) -c datatypes.c -o datatypes.o $(CFLAGS)
	$(CC) -c allocator.c -o allocator.o $(CFLAGS)
	ar rcs libdatatypes.a $(OBJS)
	$(CC) -shared -Wl,-soname,libdatatypes.so.0 -o libdatatypes-$(MAJOR_VERSION).$(MINOR_VERSION).$(PATCHLEVEL).so $(OBJ)

clean:
	rm -f *.o
	rm -f libdatatypes.a
	rm -f ./libdatatypes-$(MAJOR_VERSION).$(MINOR_VERSION).$(PATCHLEVEL).so
