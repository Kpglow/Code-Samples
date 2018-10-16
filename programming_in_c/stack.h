// stack.h
# ifndef _STACK_H
# define _STACK_H

# include <stdint.h>
# include <stdbool.h>
# include "huffman.h"

typedef treeNode *vector;

typedef struct stack
{
	uint32_t size;
	uint32_t top;	
	vector *entries;
}stack;

stack *newStack (void);

void delStack (stack *st);

vector pop (stack *st);

void push (stack *st, vector input);

bool isEmpty (stack *st);

bool isFull (stack *st);

# endif




