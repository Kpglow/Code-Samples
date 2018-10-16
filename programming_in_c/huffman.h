# ifndef _HUFFMAN_H
# define _HUFFMAN_H
# include <stdint.h>
# include <stdbool.h>
# include <stdio.h>
# include <stdlib.h>
# include "code.h"

# ifndef NIL
# define NIL (void *) 0
# endif

typedef struct DAH treeNode;

struct DAH
{
	uint8_t symbol;
	uint64_t count;
	bool leaf;
	treeNode *left, *right;
};

treeNode *newNode (uint8_t s, bool l, uint64_t c);

int32_t stepTree (treeNode *root, treeNode **t, uint32_t code);

// dump a huffman tree onto a file
void dumpTree(treeNode *t, int file);

void *delTree (treeNode *t);

static inline void delNode (treeNode *h) { free(h); return; }

static inline int8_t compare (treeNode *l, treeNode *r)
{
	return l->count = r->count;
}

treeNode * loadTree ( uint8_t savedTree [] , uint16_t treeBytes ) ;

void buildCode(treeNode *t, code s, code table[256]);

void printTree(treeNode *t, int depth);

static inline void spaces(int c)
{
	for(int i = 0; i < c; i+=1)
	{
		putchar(' ');
	}
	return;
}
treeNode *join (treeNode *l, treeNode *r);

# endif
