CC=gcc
CFLAGS=-Wall -std=c99 -O2 -g -DWITH_PTHREAD

SRCS=$(wildcard *.c)
GENSRCS=test-assocarray.c test-hashtable.c test-rbtree.c
OBJS=$(GENSRCS) $(GENSRCS:.c=) $(SRCS:.c=)

LIB=-ldatatypes-0.3.0 -pthread

.PHONY: all clean

all: $(OBJS)

%.c:%.m4
	m4 $< > $@

.c:
	$(CC) $(CFLAGS) $(INC) $< -o $@ $(LIB)

clean:
	rm -f $(OBJS)
