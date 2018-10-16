// queue.h - queue interface
// Author: Prof. Darrell Long on lab hand out

# ifndef _QUEUE_H
# define _QUEUE_H
# include <stdint.h>
# include <stdbool.h>
# include "huffman.h"

typedef treeNode item;

typedef struct queue 
{
	uint32_t size;	// how big?
	uint32_t head, tail;	// where's the top?
	item *Q;	// array to hold it
} queue;

queue *newQueue(uint32_t size);		// constructor
void delQueue(queue *q);	// destructor

bool empty(queue *q);	// is it empty?
bool full(queue *q); 	// is it full?

bool enqueue(queue *q, item i);	// add an item
bool dequeue(queue *q, item *i);	// remove from the rear


# endif
