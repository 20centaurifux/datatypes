CFLAGS = -Wall -std=c99 -O3 -DNDEBUG
all:
	gcc -c hashtable.c -o hashtable.o $(CFLAGS)
	gcc -c rbtree.c -o rbtree.o $(CFLAGS)
	gcc -c test.c -o test.o $(CFLAGS)
	gcc hashtable.o rbtree.o test.c -o hashtable $(CFLAGS)
