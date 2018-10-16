# include "huffman.h"
# include "stack.h"
# include <stdio.h>
# include <fcntl.h>
# include <unistd.h>
# include <getopt.h>
# include <assert.h>
# include <stdlib.h>
# include <errno.h>
# include <inttypes.h>
# include <string.h>
# define BUFFSZ 4

int main (int argc, char** argv)
{
	int options = 0;
	int iFlag = 0, oFlag = 0, vFlag = 0;
	int DeFile;
	char *inputFile;
 	char *outputFile;
	int pos = 0;
	//getopt function accepts -i, -o
	//implementing -v soon
	while ((options = getopt(argc,argv,"vi:o:")) != -1)
	{
		++pos;
		switch(options){
		case 'i':
			iFlag = 1;
			inputFile = argv[++pos];
			//strcpy(inputFile, optarg);
			break;
		case 'o':
			oFlag = 1;
			outputFile = argv[++pos];
			break;
		case 'v':
			vFlag = 1;
			break;
		default :
			exit(1);
		}	
	}
	uint8_t *savedTree; // array/pointer to a group of uint8_t bytes that will store the symbols for the huffman tree as well as L for leaf and I for Interior node
	uint32_t magic = 0; // magic = 0xdeadd00d
	uint64_t size = 0; // size = number of bytes in the file
	uint16_t treeSize = 0; // treeSize = number of bytes allocated for the huffman tree
	//DeFile = open(inputFile, O_RDONLY);
	if (iFlag == 0)
	{// if -i is not turned on then the user can not select an input file
		//close(DeFile);
		printf("turn on -i to accept inputfile\n");
		return 0;
	}
	DeFile = open(inputFile, O_RDONLY);
	if(DeFile == -1)
	{
		printf("Input file Error\n");
		return 0;
	}
	read(DeFile, &magic, sizeof magic);

	if (magic != 0xdeadd00d)
	{ // if the magic number for the file does not equal 0xdeadd00d then it is not the specified compressed file to decode
		printf("error, not a compressed file\n");
		printf("%u = is the not the magic number\n", magic);
		close(DeFile); // since we have opened the file and is currently reading it, we must close the file before we return
		return 0;
	}
	read(DeFile, &size, sizeof size); // next store the unsigned 64 bit integer into size for later debugging purposes/ # of bytes needed for the array of characters in the output file
	read(DeFile, &treeSize, sizeof treeSize);// then we store the treeSize which is the amount 2 bytes to determine how big the tree is going to be
	savedTree = (uint8_t*)calloc(treeSize,sizeof(uint8_t)); // here we allocate memory, and since it is using calloc we can say we need treeSize * sizeof(uin8_t) since we need that many bytes for storing the tree
	uint8_t savedHere = 0;
	for (uint16_t i = 0; i < treeSize; i++)
	{ // here we read in the compressed version of the tree and store it into the array from 0 -> treeSize as indexes
		if (read(DeFile, &savedHere, sizeof savedHere) > 0)
		{ // using the temporary pointer 'savedHere' to set the i'th position of the array to be that tmp pointer's address
			savedTree[i] = savedHere;
		} 
	}
	treeNode *root = loadTree(savedTree, treeSize); // loads the tree
	treeNode *test[2]; // technically a double pointer that will store the temp location t[0] of the tree and if it is a leaf node, then we store tmp location back to the static location which is t[1]
	test[0] = root; 
	test[1] = root;

	int fd = -1;
	if (oFlag == 1)
	{
		fd = open(outputFile, O_WRONLY | O_CREAT | O_EXCL, 0644); // opens the output file, permissions-flags etc
	}
	if ((fd == -1) && (EEXIST == errno))
	{ // checks for output file, if it exists then just overwrite it no need to create it again
		perror("Output Error");
		delTree(root);
		free(savedTree);	
		exit(errno);
	}
	int bit = 0;
	char currentChar;
	int32_t checkBit;
	char *letters;
	int k = 0;
	letters = (char *)malloc((size*sizeof(char))+1);
	letters[size*sizeof(char)] = '\0';
	for (uint32_t j = 0; j < size; j++)
	{
		if (read(DeFile, &currentChar, sizeof(char)) > 0)
		{
			for(int i = 0; i < 8; i++)
			{
				bit = (currentChar & (0x1 << ( i % 8))) >> ( i % 8 );
				if ((checkBit=stepTree(root,test,bit))!=-1)
				{
					letters[k] = checkBit;
					k++;
				}
			}
		}
	}
	if (vFlag == 1)
	{ // size is the amount of bytes, to get bits we * it by 8
		printf("Original %lu bits: tree(%u)\n", size*8, treeSize);
	}
	if (oFlag == 1)
	{ // if the -o flag is on then write it onto the output file
		write(fd,letters,size);
	}else
	{ // if the -o flag is not turned on then write it onto cmd prompt
		for (uint64_t std = 0; std < size; std++)
		{ // printf the array instead
			printf("%c", letters[std]);
		}
	}
	if (fd != -1)
	{ // if the output file exists meaning -o flag was on then we close the output file
		close(fd);
	}
	free(letters);
	if (DeFile != -1 && iFlag == 1)
	{ // if the input file exists meaning -i flag was on then we close the input file
		close(DeFile);
	}
	delTree(root); // recursively deletes the huffman tree starting at the root
	free(savedTree); // frees the allocated memory for the savedTree array
	return 0;	
}
