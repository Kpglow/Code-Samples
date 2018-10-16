// encode.c
# include <stdio.h>
# include <stdint.h>
# include <stdlib.h>
# include <stdbool.h>
# include <fcntl.h>
# include <unistd.h>
# include <ctype.h>
# include <getopt.h>
# include <errno.h>
# include "queue.h"
# include "huffman.h"
# include "code.h"
# include "stack.h"
# include "bv.h"

int main( int argc, char **argv )
{
	// flags
	char *inputFile = NULL;
	char *outputFile = "output.txt";
	int verbose = 0, print = 0;
	int c, filedescIN, filedescOUT;
	
	// read in (stdin / source file) and make histogram
	uint32_t histogram[256];	
	for (int i = 0; i < 256; i++) 
	{ 
		histogram[i] = 0; 
	}
	histogram[0] = 1;			
	histogram[255] = 1;
	uint64_t bytes = 0;			// bytes in the source file
	char data[1];	
	
	while ((c = getopt(argc, argv, "i:o:vp")) != -1)	
	{
		switch (c)	
		{
			case 'i':	// input
				inputFile = optarg;
				filedescIN = open(inputFile, O_RDONLY);	// open source file ...
				if (filedescIN < 0)	// error checker ...
				{
					printf("error file open\n");
					return 1;
				}
				while (read(filedescIN, data, 1) != 0)	// read to histogram until EOF
				{
					bytes++;
					histogram[ (uint32_t)data[0] ] += 1;
				}
	
				if ( close(filedescIN) < 0 )	// close the file
				{
					printf("error close\n");
					return 1;
				}
				break;
				
			case 'o':	// output
				outputFile = optarg;
				break;
				
			case 'v':	// verbose
				verbose = 1;
				break;
				
			case 'p':	// print flag 
				print = 1;
				break;
				
			default: 
				exit(EXIT_FAILURE);
		}
	}
	
	uint64_t stdSize = 256;
	char *stdInput = (char *)calloc(stdSize, sizeof(char)); 
	if (inputFile == NULL)
	{
		while (read(0, data, 1) != 0)	// read to histogram until EOF
		{
			stdInput[ bytes++ ] = data[0];
			histogram[ (uint32_t)data[0] ] += 1;
			if (bytes == stdSize)
			{
				stdSize = stdSize << 1;
				stdInput = (char *)realloc(stdInput, sizeof(char)*stdSize);
			}
		}
		printf("\n");
	} 
	
	
	// make the treenode and put into priority queue
	uint32_t leafCount = 0;
	queue *test = newQueue(256);
	treeNode *tree[256];
	for (int i = 0; i < 256; i++)		// read through the histogram and make treeNode
	{
		if (histogram[i] > 0)
		{
			leafCount++;
			tree[i] = newNode(i, true, histogram[i]);
			if (!enqueue(test, *tree[i]))	// priority enqueue
			{
				printf("failed enqueue\n");
			}
		}
	}
	
	// make the huffman tree with the priority queue
	uint32_t totalLeaf = leafCount;			// leaf count
	
	treeNode *small[ totalLeaf-1 ];		// initialize the blocks for the tree
	treeNode *nextsmall[ totalLeaf-1 ];
	treeNode *top[ totalLeaf-1 ]; 

	for (uint32_t i = 0; i < (totalLeaf-1); i++)
	{
		small[i]= (treeNode *)calloc(1, sizeof(treeNode));
		nextsmall[i] = (treeNode *)calloc(1, sizeof(treeNode));
	}
	int smallIndex = 0;
	uint32_t lastIndex;
	while( leafCount > 1 )
	{
		(void)dequeue(test, small[ smallIndex ]);
		(void)dequeue(test, nextsmall[ smallIndex ]);
	
		top[ smallIndex ] = join(small[ smallIndex ], nextsmall[ smallIndex ]);
		
		(void)enqueue(test, *top[ smallIndex ]);
		lastIndex = smallIndex;
		smallIndex++;
		leafCount--;
	}
	
	// build code and bit vector
	bitV *vector[256];	// initialize vectors 
	code table[256];	// initialize code table
	for (int i = 0; i < 256; i++)
	{
		table[i] = newCode();
	}
	code codex = newCode();
	buildCode(top[lastIndex], codex, table);	// get the huffman tree code
	uint32_t popped;
	for (int i = 0; i < 256; i++)
	{
		if ( !emptyCode(&table[i]) )
		{
			vector[i] = newVec( table[i].l );
			uint32_t counter = 0;
			while ( popCode(&table[i], &popped) )
			{
				if (popped == 1)
				{
					setBit(vector[i], counter);
				}
				counter++;
			}
		}
	}
	
	
	filedescOUT = open(outputFile, O_WRONLY | O_CREAT | O_EXCL, 0644);	// open output file
	if (filedescOUT < 0 && (EEXIST == errno))	// error checker ...
	{
		perror("output file error");
		free( stdInput );
		for (int i = 0; i < 256; i++)
		{
			if (histogram[i] > 0)
			{
				delTree( tree[i] ); 	// free treeNode
				delVec( vector[i] );	// free bit vectors
			}
		}
		if ((totalLeaf-2) == 0)
		{
			delTree( top[0] );	// free only the first if nothing in file
		}
		else
		{
			for (uint32_t i = 0; i < (totalLeaf-1); i++)	// post order deletion
			{
				delNode( top[i]->left );
				delNode( top[i]->right );
				delNode( top[i] );
			}
		}
		delQueue( test );		// free queue
		exit(errno);
	}
	
	
	uint32_t magic[1];		// write the 4 bytes magic number
	*magic = 0xdeadd00d;
	write(filedescOUT, magic, 4);
	
	uint64_t bits[1];		// write the 8 bytes total bytes of source file
	*bits = bytes;
	write(filedescOUT, bits, 8);
	
	uint16_t treeSize[1];	// write the 2 bytes total size of tree
	*treeSize = ( (3 * totalLeaf) - 1 );
	write(filedescOUT, treeSize, 2);
	
	dumpTree( top[lastIndex], filedescOUT );	// write post-traversal list of huffman tree
	
	
	// Encode the contents of the source and write to output file
	uint32_t totalBits = 0;		// the total bits converted
	char convert[1];			// buffer hold (read source)
	uint8_t pasteCode[1];		// buffer hold (write encode)
	uint8_t encoded = 0;		// 8 bits to encode
	int numSet = 0;
	if (inputFile)	// source file
	{
		filedescIN = open(inputFile, O_RDONLY);	// open file ...
		if (filedescIN < 0)	// error checker ...
		{
			printf("error file open\n");
			return 1;
		}
		
		while (read(filedescIN, convert, 1) != 0)	// go through the entire source file
		{
			for (int j = (lenVec(vector[ (uint32_t)convert[0] ]) - 1); j >= 0; j--)	// loop through the bit vector
			{
				numSet++;
				totalBits++;
				if ( valBit(vector[ (uint32_t)convert[0] ], j) == 1 )	// if "1" set 1, if not leave it 0
				{
					encoded |= (0x80);
				}
				
				if (numSet >= 8)	// if the 8 bits are ready
				{
					*pasteCode = encoded;
					write(filedescOUT, pasteCode, 1);	// write the 8 encoded bits to output file
					encoded = 0;					// reset count and 8 bits
					numSet = 0;
				}
				else 
				{
					encoded = (encoded >> 1);		// shift bits by 1 left if 8 bits are not ready yet
				}
			}	
		}
		if (numSet > 0)	// if left bits are still there 
		{
			encoded =  encoded >> (8 - (numSet + 1));	// shif rest of the bits to the left end
			*pasteCode = encoded;
			write(filedescOUT, pasteCode, 1);			// write the rest of hte bits to the output file
		}
		if ( close(filedescIN) < 0 )	// close the input file
		{
			printf("error close\n");
			return 1;
		}
	} 
	else		// standard input
	{
		for (uint32_t i = 0; i < bytes; i++)
		{
			for (int j = (lenVec(vector[ (uint32_t)stdInput[i] ]) - 1); j >= 0; j--)	// loop through the bit vector
			{
				numSet++;
				totalBits++;
				if ( valBit(vector[ (uint32_t)stdInput[i] ], j) == 1 )	// if "1" set 1, if not leave it 0
				{
					encoded |= (0x80);
				}
				
				if (numSet >= 8)	// if the 8 bits are ready
				{
					*pasteCode = encoded;
					write(filedescOUT, pasteCode, 1);	// write the 8 encoded bits to output file
					encoded = 0;					// reset count and 8 bits
					numSet = 0;
				}
				else
				{
					encoded = (encoded >> 1);		// shift bits by 1 left if 8 bits are not ready yet
				}
			}	
		}
		
		if (numSet > 0)	// if left bits are still there
		{
			encoded = encoded >> (8-(numSet + 1)); // shift rest of the bits to the left end
			*pasteCode = encoded; //encoded;
			write(filedescOUT, pasteCode, 1);// write the rest of the bits to the output file		           
		}
	}
	
	if ( close(filedescOUT) < 0 )	// close the output file
	{
		printf("error close\n");
		return 1;
	}
	
	

	
	if (verbose)		// if verbose flag is set, print stats
	{
		bytes *= 8;
		printf("Original %lu bits: leaves %u (%u bytes)", bytes, totalLeaf, treeSize[0]);
		double percentage = 0; 
		if (bytes != 0)
		{
			percentage = (double)(100)*((double)totalBits / (double)bytes);
		}
		printf(" encoding %u bits (%.4f%%)\n", totalBits, percentage);
	}
	if (print)			// if print flag is set, print the huffman tree
	{
		printTree(top[lastIndex], totalLeaf-1);
	}
	
	
	// free all 
	free( stdInput );
	for (int i = 0; i < 256; i++)
	{
		if (histogram[i] > 0)
		{
			delTree( tree[i] ); 	// free treeNode
			delVec( vector[i] );	// free bit vectors
		}
	}
	if ((totalLeaf-2) == 0)
	{
		delTree( top[0] );	// free only the first if nothing in file
	}
	else
	{
		for (uint32_t i = 0; i < (totalLeaf-1); i++)	// post order deletion
		{
			delNode( top[i]->left );
			delNode( top[i]->right );
			delNode( top[i] );
		}
	}
	delQueue( test );		// free queue
	return 0;
}
