//File:   bv.c
// library
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "bv.h"

bitV *newVec(uint32_t length)	// create a new vector of length
{
	bitV *vector = (bitV *)calloc(1, sizeof(bitV));	// make one vector
	vector->v = (uint8_t *)calloc( ((length >> 3) + 1), sizeof(uint8_t)); // length bits
	vector->l = length; // total length
	return vector;
}

void delVec(bitV *vector)	// deletes a vector
{
	free( vector->v );		// free up the space for bit vector
	free( vector );			// free up the space for vector
	return;
}

void oneVec(bitV *vector)	// Creates a vector of all 1
{
	for(uint32_t count = 0; count <= lenVec(vector); count++)
	{
		setBit(vector, count);	// go through all vector bit and set 1
	}
	return;
}

void setBit(bitV *vector, uint32_t set) // sets a specified bit
{
	vector->v[set >> 3] |= (0x01 << (set & 0x07)); // inspired from lecture
	return;	// divide set with 8 to find the byte space and OR the bit 
}

void clrBit(bitV *vector, uint32_t set) // clears a specified bit
{
	vector->v[set >> 3] &= ~(0x01 << (set & 0x07)); // inspired from lecture
	return;	// divede set with 8 to find the byte space and AND the bit
}

uint8_t valBit(bitV *vector, uint32_t set) // return the value of bit
{
	uint8_t vBit;
	vBit = (vector->v[set >> 3] & (0x01 << (set&0x07))) >> (set&0x07);
	return vBit;	// get the bit from the byte space based on set 
}

uint32_t lenVec(bitV *vector) // return the length of the vector
{
	return vector->l; // return the vector length
}



