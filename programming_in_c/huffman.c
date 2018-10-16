// huffman.c
# include <stdint.h>
# include <stdbool.h>
# include <stdlib.h>
# include <unistd.h>
# include <fcntl.h>
# include <stdio.h>
# include <ctype.h>
# include "huffman.h"
# include "code.h"
# include "stack.h"

treeNode *newNode (uint8_t s, bool l, uint64_t c)
{
	treeNode *newTree = (treeNode*)calloc(1, sizeof(treeNode));
	if (newTree)
	{
		newTree->symbol = s;
		newTree->leaf = l;
		newTree->count = c;
		newTree->left = NIL;
		newTree->right =NIL;
		return newTree;
	}
	free( newTree );
	return (treeNode *) 0;
}
void dumpTree(treeNode *t, int file)
{
	if(t != NIL)
	{
		dumpTree(t->left, file);	// go left
		dumpTree(t->right, file);	// go right
		if(t->leaf)
		{
			write(file, "L", 1);	// write "L" and symbol
			uint8_t sym[1];
			*sym = t->symbol;
			write(file, sym, 1);
		}
		else
		{
			write(file, "I", 1);	// write "I" for internal node
		}
	}
	return;
}

int32_t stepTree (treeNode *root, treeNode **t, uint32_t code)
{
	root = t[0];
	if (root)
	{	
		if (code == 1)
		{
			if (root->right->leaf)
			{
				t[0] = t[1];
				return root->right->symbol;
			}
			t[0] = root->right;
			return -1;
		} else if (code == 0)
		{
			if (root->left->leaf)
			{
				t[0] = t[1];
				return root->left->symbol;
			}
			t[0] = root->left;
			return -1;
		}
	}
	return -1;
}

void buildCode(treeNode *t, code s, code table[256])
{
	if (t->leaf)
	{
		table[ t->symbol ] = s;	// save the code to code table
	}
	else 
	{
		if ( pushCode(&s, 0) )
		{
			buildCode(t->left, s, table);
			uint32_t popped;
			popCode(&s, &popped);
		}
		if ( pushCode(&s, 1) ) 
		{
			buildCode(t->right, s, table);
			uint32_t popped;
			popCode(&s, &popped);
		}
	}

}


treeNode * loadTree ( uint8_t savedTree [] , uint16_t treeSize )
{
	stack *tree = newStack();
	treeNode *node;
	treeNode *leftChild;
	treeNode *rightChild;
	for (uint16_t i = 0; i < treeSize; i++)
	{ // looking at the array that is recently stored in the previous instructions, using if-statements to determine if the treeNode is a leaf or interior node based off of the compressed data
		if (savedTree[i] == 'L')
		{ // determined that the current node is a leaf and should push the node onto the stack
			//printf("leaf\n");
			i++;
			node = newNode(savedTree[i],true,0);
			push(tree, node);
			continue;
		}
		if (savedTree[i] == 'I')
		{ // determined that the current node is an interior node and that we should pop the right and left child in that order and join it into one node and push it back onto the stack
			rightChild = pop(tree);
			leftChild = pop(tree);
			treeNode *joined = join(leftChild,rightChild);
			push(tree, joined);
		}
	}
	treeNode *root = pop(tree); // pops one last time to get the location of the root of the huffman tree
	delStack(tree);
	return root;
}
void *delTree (treeNode *t)
{
	if (t)
	{
		delTree(t->left);
		delTree(t->right);
		delNode(t);
	}
	return (void *)0;
}

treeNode *join (treeNode *l, treeNode *r)
{
	treeNode *joined = newNode('$', false, (l->count) + (r->count));
	joined->left = l;
	joined->right = r;
	return joined;
}

void printTree(treeNode *t, int depth)
{
	if (t)
	{
		printTree(t->left, depth + 1); // go left
		if ( t->leaf ) // if treeNode is leaf
		{
			if (isalnum(t->symbol))
			{
				spaces(4 * depth);
				printf("'%c' (%lu)\n", t->symbol, t->count);
			}
			else
			{
				spaces(4 * depth);
				printf("0x%x (%lu)\n", t->symbol, t->count);
			}
		}
		else
		{
			spaces(4 * depth);
			printf("$ (%lu)\n", t->count);
		}
		printTree(t->right, depth + 1);
	}
	return;
}

