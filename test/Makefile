CC=gcc
CFLAGS=-Wall -std=c99 -O0 -g -DWITH_PTHREAD

SRCS=$(wildcard *.c)

INC=-I..
LIB=-L.. -ldatatypes-0.2.0 -pthread

.PHONY: all clean

all: $(SRCS:.c=)

.c:
	$(CC) $(CFLAGS) $(INC) $< -o $@ $(LIB)

clean:
	ls ./*.c | while read f; do rm -f "$${f::(-2)}"; done
