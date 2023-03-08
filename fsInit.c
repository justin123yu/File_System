/**************************************************************
* Class:  CSC-415-0# Fall 2021
* Names: Justin Yu, Wen Hao Li, Joycelyn Chan, Austin Nguyen
* Student IDs:920536640, 921218295, 920369317, 921846377
* GitHub Name:justin123yu, careit, joycelynchan, popcorneatr
* Group Name:JJAK
* Project: Basic File System
*
* File: fsInit.c
*
* Description: Main driver for file system assignment.
*
* This file is where you will start and initialize your system
*
**************************************************************/


#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include "fsLow.h"
#include "mfs.h"
#include "FreeSpace.h"
#include "DEntry.h"
#include "Vcb.h"
#define  magicNumber 420

//Make sure to move all the typdef struct into a .h file



TheFreeSpaceMap map;

//Global bitmap
unsigned char* bitmap;

//Global VCB
VCB* vcbpointer;

	//starts at 0
int bitCounter(unsigned char target){
		int x = 0;
		if((target & 0x80) == 0x80) ++x; 
		if((target & 0x40) == 0x40) ++x; 
		if((target & 0x20) == 0x20) ++x;
		if((target & 0x10) == 0x10) ++x;
		if((target & 0x08) == 0x08) ++x;
		if((target & 0x04) == 0x04) ++x;
		if((target & 0x02) == 0x02) ++x;
		if((target & 0x01) == 0x01) ++x;
		return x;
}


//Sets the bit based on location.
//https://www.geeksforgeeks.org/set-k-th-bit-given-number/ helped explain the logic for setting bits.
void setMutipleBit(int count)
{   
	int newCount = count - 1;
	int remaining;
	if (newCount + map.bitLocation > 7){
		remaining = 7 % map.bitLocation;
		for(int i = map.bitLocation; i <= remaining + map.bitLocation; i++){
			bitmap[map.index] |= 1 << i;
		}
		map.bitLocation = 0;
		map.index += 1;
		newCount = newCount - 1; 
		remaining = 0;
	}
	if (newCount + map.bitLocation <= 7){
		for(int i = map.bitLocation; i <= newCount + map.bitLocation; i++){
			bitmap[map.index] |= 1 << i;
		}
	}
	map.bitLocation = newCount + 1;
	map.freeBits += count;
	LBAwrite(bitmap,5,1);
}	


	//Handles Free Space
int initFreeSpace(int blockSize, int numberOfBlocks, int blocksNeeded){
	setMutipleBit(6);
	//returns next bit
	LBAwrite(bitmap, blocksNeeded, 1);
	return map.freeBits;
}

//initalizing VCB
void initVCB(VCB* vcb,int blockSize, int numberOfBlocks, int blocksNeeded){
		vcb->blockSize = blockSize;
		vcb->startPostion = 0;
		vcb->rootLocation = 0;
		vcb->freeBlockLocation = initFreeSpace(blockSize, numberOfBlocks, blocksNeeded);
}

int rootDirectory(int sizeOfBlock){
	int blockSize = sizeOfBlock;
	int returnBlock = 0;
	int tmpEntries = 58;
	int bytesNeeded = sizeof(DEntry) * tmpEntries;
	DEntry* rootPtr = malloc(bytesNeeded);
	int blocksNeeded = ((bytesNeeded+(blockSize-1))/ blockSize);
	vcbpointer->sizeOfDirectory = blocksNeeded * blockSize;
	vcbpointer->blocksToRead = blocksNeeded;
	for(int i = 0; i < tmpEntries; i++){
		rootPtr[i].freeState = 0;
	}
	strcpy(rootPtr[0].name,".");
	rootPtr[0].size = blocksNeeded * blockSize;
	rootPtr[0].returnBlock = map.freeBits;
	rootPtr[0].blockToRead = blocksNeeded;
	rootPtr[0].freeState = 1;
	rootPtr[0].type = 1;
	strcpy(rootPtr[1].name,"..");
	rootPtr[1].returnBlock = rootPtr[0].returnBlock;
	rootPtr[1].size = rootPtr[0].size;
	rootPtr[1].blockToRead = blocksNeeded;
	rootPtr[1].freeState = 1;
	rootPtr[1].type = 1;
	setMutipleBit(blocksNeeded);
	LBAwrite(rootPtr,rootPtr[1].blockToRead,rootPtr[1].returnBlock);
	returnBlock = rootPtr[1].returnBlock;
	free(rootPtr);
	return returnBlock;
}

//Used to initalize the MAP
void initDirectoryMap(){
	int index = 0;
	int bitLocation = bitCounter(bitmap[index]);
	int totalBits = bitCounter(bitmap[index]);
	while(bitLocation == 8){
		index = index + 1;
		bitLocation = bitCounter(bitmap[index]);
		totalBits += bitLocation;
	}
	map.freeBits = totalBits -1;
	map.bitLocation = bitLocation;
	map.index = index;
}

int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */
	vcbpointer = malloc(blockSize);
	int NumBytesNeeded = (numberOfBlocks + (8 - 1)) / 8;
	int BlocksNeeded = (NumBytesNeeded + (blockSize - 1)) / blockSize;
	LBAread(vcbpointer,1,0);
	if(vcbpointer->magicSignature != magicNumber ){
		bitmap = malloc(BlocksNeeded * blockSize);
		initVCB(vcbpointer,blockSize, numberOfBlocks,BlocksNeeded);
		vcbpointer->rootLocation = rootDirectory(blockSize);
		vcbpointer->magicSignature = magicNumber;
		LBAwrite(vcbpointer,1,0);
	}
	if(bitmap == NULL){
		bitmap = malloc(BlocksNeeded * blockSize);
		LBAread(bitmap,BlocksNeeded,1);
		initDirectoryMap();
		// printf("\n MAP information map Free Bit: [%d], map BitLocation [%d], map Index = [%d] \n ",	map.freeBits, map.bitLocation, map.index);
	}
	// int result = fs_setcwd("apple");
	// printf("\n RESULT [%d]\n", result);
	// printf("\nBEGINGING MKDIR\n");
	// int result2 = fs_mkdir("test",1);
	// printf("\n END OF MAKING DIR\n");
	// printf("\n RESULT2 [%d]\n", result2);
	// char tmp[25];
	// strcpy(tmp,"apple/./..");
	// printf("\n [%s] HELLO WORLD\n ",fs_getcwd(tmp, strlen(tmp)+1));
	return 0;
	}
	
	
void exitFileSystem ()
	{
		printf ("System exiting\n");
		free(bitmap);
	}
