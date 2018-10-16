// bv.h - Bit Vector interface
// Author: Prof. Darrell Long

# ifndef _BVector
# define _BVector
# include <stdint.h>

typedef struct bitV {
	uint8_t *v;
	uint32_t l;
} bitV;

bitV *newVec(uint32_t);	// Create a new vector of specified length

void delVec(bitV *);	// Deletes a vector

void oneVec(bitV *);	// Creates a vector of all 1

void setBit(bitV *, uint32_t);	// Sets a specified bit

void clrBit(bitV *, uint32_t);	// Clears a specified bit

uint8_t valBit(bitV *, uint32_t); // Returns the value of a specified bit

uint32_t lenVec(bitV *); // Return the length of the vector
# endif
