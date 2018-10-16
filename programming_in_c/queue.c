// queue.c
# include <stdint.h>
# include <stdbool.h>
# include <stdlib.h>
# include <stdio.h>
# include "queue.h"
# include "huffman.h"

queue *newQueue(uint32_t size)
{
	queue *newQ = (queue *)calloc(1, sizeof(queue)); // make a queue
	if (newQ)
	{
		newQ->size = size;	// save the size of queue
		newQ->head = 0;	// set the head as 0
		newQ->tail = 0;	// set the tail as 0
		newQ->Q = (item *)calloc( size, sizeof(item)); 
		return newQ;
	}
	free( newQ );	// free when calloc fails
	return (queue *) 0;
}

void delQueue(queue *q)
{
	
	free( q->Q );		// free the item inside queue
	free( q );			// free queue
	return;
}

bool empty(queue *q)	// checks empty
{
	if (q->head == q->tail)		// is the head in the beginnning?
	{
		return true;
	}
	return false;
}

bool full(queue *q)		// checks full 
{
	if ((q->head == (q->size - 1) && q->tail == 0) || (q->tail == (q->head + 1)))	// has the head reached the size?
	{	
		return true;
	}
	return false;
}

bool enqueue(queue *q, item i)	// add an item according to order
{
	if (!full(q))
	{
		if (empty(q))	// if the queue is push for the first time
		{
			q->Q[ (q->head) ] = i;
			q->head += 1;
			q->head %= (q->size);
			return true;
		} 
		else 
		{
			int inserted = 0;			// check for insertion
			uint32_t index = q->tail;	// current index
			while (!inserted)	// keep looking for the index to put in
			{
				if ((i.count) <= q->Q[index].count || index == q->head)
				{
					inserted = 1;
				} 
				else 
				{
					index++;
					index %= (q->size);
				}
			}
			uint32_t trav = (q->head + 1) % (q->size);	// shift all right
			while (index < trav)	// shift the queue to right place
			{
				if( trav == 0)
				{
					trav = (q->size-1);
				}
				q->Q[trav] = q->Q[trav-1];
				trav--;
			}
			q->Q[index] = i;	// set new queue
			q->head += 1;
			q->head %= (q->size);
			return true;
		}
	}
	return false;
}

bool dequeue(queue *q, item *i)
{
	if (!empty(q))	// check if the queue is not empty
	{
		*i = (q->Q[ q->tail ]);	// return pointer of dequeue item
		q->tail += 1;	// set new queue tail pointer 
		q->tail %= (q->size);
		return true;
	}
	return false;
}
