CC=gcc
CFLAGS=-Wall -std=c99 -O0 -g $(PTHREAD_CFLAGS) -DWITH_PTHREAD

SRCS=$(wildcard *.c)

INC=-I..
LIB=-L.. -ldatatypes-0.1.0

.PHONY: all clean

all: $(SRCS:.c=)

.c:
	$(CC) $(CFLAGS) $(INC) $< -o $@ $(LIB)

clean:
	ls ./*.c | while read f; do rm -f "$${f::(-2)}"; done
