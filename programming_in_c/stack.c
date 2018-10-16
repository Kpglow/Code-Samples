// stack.c
# include <stdint.h>
# include <stdbool.h>
# include <stdlib.h>
# include <stdio.h>
# include "stack.h"
# include "huffman.h"

stack *newStack(void)
{
	stack *newS = (stack *)calloc(1, sizeof(stack));
	if (newS)
	{
		newS->size = 1;
		newS->top = 1;
		newS->entries = (vector *)calloc(newS->size, sizeof(vector));
		return newS;
	}
	free( newS );
	return (stack *) 0;
}

void delStack(stack *st)
{
	free(st->entries);
	free(st);
}

vector pop(stack *st)
{
	if(!isEmpty(st))
	{
		st->top -= 1;
		return st->entries[ st->top ];
	}
	else
	{
		return NULL;
	}
}

void push(stack *st, vector input)
{
	if (isFull(st))
	{
		st->size *= 2;
		st->entries = (vector *)realloc(st->entries, sizeof(vector) * st->size);
		st->entries[ (st->top) ] = input;
		st->top += 1;
		return; 
	} 
	st->entries[ (st->top) ] = input;
	st->top += 1;
	return;
}

bool isEmpty(stack *st)
{
	if (st->top == 0)
	{
		return(1);
	}else
	{
		return(0);
	}
}
bool isFull(stack *st)
{
	if(st->top < st->size)
	{
		return false;
	}else
	{
		return true;
	}
}



