CFLAGS = -Wall -std=c99 -O3 -DPTHREADS -DNDEBUG
all:
	gcc -c hashtable.c -o hashtable.o $(CFLAGS)
	gcc -c rbtree.c -o rbtree.o $(CFLAGS)
	gcc -c list.c -o list.o $(CFLAGS)
	gcc -c datatypes.c -o datatypes.o $(CFLAGS)
	gcc -c allocator.c -o allocator.o $(CFLAGS)
	gcc -c test.c -o test.o $(CFLAGS)
	gcc hashtable.o rbtree.o test.o list.o datatypes.o allocator.o -o hashtable $(CFLAGS)

clean:
	rm *.o
	rm -f hashtable
	rm -f hashtable.exe
