CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Werror -std=c99
OBJECTS = decode.o encode.o queue.o stack.o huffman.o bv.o
.PHONY: all
all: encode decode queue.o stack.o huffman.o bv.o

encode	:	$(OBJECTS)
	$(CC) $(CFLAGS) -o encode encode.o stack.o huffman.o queue.o bv.o
decode	:	$(OBJECTS)
	$(CC) $(CFLAGS) -o decode decode.o stack.o huffman.o
encode.o :	encode.c
	$(CC) $(CFLAGS) -c encode.c
decode.o :	decode.c
	$(CC) $(CFLAGS) -c decode.c
queue.o	:	queue.c
	$(CC) $(CFLAGS) -c queue.c
stack.o	:	stack.c
	$(CC) $(CFLAGS) -c stack.c
huffman.o :	huffman.c
	$(CC) $(CFLAGS) -c huffman.c
bv.o	:	bv.c
	$(CC) $(CFLAGS) -c bv.c
.PHONY: clean
clean:
	rm $(OBJECTS) encode decode
