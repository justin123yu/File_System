/**************************************************************
* Class:  CSC-415-0# Fall 2021
* Names: Justin Yu, Wen Hao Li, Joycelyn Chan, Austin Nguyen
* Student IDs:920536640, 921218295, 920369317, 921846377
* GitHub Name:justin123yu, careit, joycelynchan, popcorneatr
* Group Name:JJAK
* Project: Basic File System
*
* File: mfs.c
*
* Description: Parse the given string path
*
**************************************************************/
//Make sure to describe the structure in a header file and put the .h into the .c files
//If im using any global pointer include extern.
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include "mfs.h"
#include "DEntry.h"
#include "FreeSpace.h"
#include "Vcb.h"
#include "fsLow.h"
#define  MaxPathSize 255


typedef struct Directory{
	//checks where our directory or file location is at and if it exist.
	int indexLocation;
	//Builds the path name
	char pathName[MaxPathSize];
}Directory;

Directory dir;



// FIX LOAD DIRECTORY PLZ
DEntry* loadDirectory(DEntry entry){
	DEntry* childDir = NULL;
	if(entry.type == 1){
		childDir = malloc(entry.size);
		int result = LBAread(childDir,entry.blockToRead,entry.returnBlock);
	}

	return childDir;

}

//Takes path and locates the directory.
DEntry parsePath(const char* path)
{
	DEntry dirEntry;
	dir.indexLocation = -1;
	DEntry* directoryPtr = malloc(vcbpointer->sizeOfDirectory);
	char* editPath = malloc(strlen(path)+1);
	int pathOffset = 0;
	strcpy(editPath , path);
	char* token = strtok(editPath,"/");
	//absolute path 
	if (editPath[0] == '/')
	{
		//tokenize the path
		while (token != NULL)
		{
			printf("\n WE IN PARSEPATH\n");
			printf("\n [%s]\n ", token);
			int result = LBAread(directoryPtr,vcbpointer->blocksToRead,vcbpointer->rootLocation);
			//loops through all the directory entry
			//instead of using 56, grow it as you make a directory or use more space.
			for (int i = 0; i < 58; i++)
			{
				//checks if token matches the current directory entry and is a directory
				//Parse Path should just tell us the location of the directory before the last element since we don't care about the last element

				//If its free skip it
				if(directoryPtr[i].freeState == 0){
					continue;
				}
				if ((strcmp(directoryPtr[i].name, token) == 0))
				{
					dir.indexLocation = i;
					//Creates the absoulte Path. Might make a function to handle this instead
					strcpy(dir.pathName + pathOffset,"/");
					pathOffset = strlen(dir.pathName);
					strcpy(dir.pathName + pathOffset ,directoryPtr[i].name);
					pathOffset = strlen(dir.pathName);
					//--------------------------------
					DEntry * tmp = loadDirectory(directoryPtr[i]);
					directoryPtr = memcpy(directoryPtr,tmp,directoryPtr[i].size);
					dirEntry = directoryPtr[i];
					free(tmp);
					break;
				}
				else {
					dir.indexLocation = -1;
				}
				
			}
			//gets next token from path(parameter)
			token = strtok(NULL, "/");
		}
	}

	//relative path
	else {
		while (token != NULL)
		{
			printf("\n [%s]\n ", token);
			//Checks if the directory is initalize or not.
			if(dir.indexLocation == -1){
				LBAread(directoryPtr,vcbpointer->blocksToRead,vcbpointer->rootLocation);
			}
			else {
				DEntry* tmp = loadDirectory(directoryPtr[dir.indexLocation]);
				directoryPtr = memcpy(directoryPtr,tmp,tmp[dir.indexLocation].size);
				free(tmp);
			}
			
			//loops through all the directory entry
			for (int i = 0; i < 58; i++)
			{
				if(directoryPtr[i].freeState == 0){
					continue;
				}
				//checks if token matches the current directory entry and is a directory
				if ((strcmp(directoryPtr[i].name, token) == 0))
				{
					dir.indexLocation = i;
					//Creates the absoulte Path. Might make a function to handle this instead
					strcpy(dir.pathName + pathOffset,"/");
					pathOffset = strlen(dir.pathName);
					strcpy(dir.pathName + pathOffset ,directoryPtr[i].name);
					pathOffset = strlen(dir.pathName);
					//---------------------------------------
					DEntry * tmp = loadDirectory(directoryPtr[i]);
					directoryPtr = memcpy(directoryPtr,tmp,directoryPtr[i].size);
					dirEntry = directoryPtr[i];
					free(tmp);
					break;
				}
				else {
					dir.indexLocation = -1;
				}
				
			}
			//gets next token from path(parameter)
			token = strtok(NULL, "/");
		}
	}
	pathOffset = 0;
	free(directoryPtr);
	free(editPath);
	return dirEntry;
} 
int freeDirectory(DEntry* directory){
	int index = -1;
	for(int i = 3; i < 58; i++){
		if(directory[i].freeState == 0){
			index = i;
		}
	}
	return index;
}

int fs_mkdir(const char *pathname, mode_t mode) {
	// Check to make sure directory doesn't already exist
	DEntry dEntry;
	printf("\n MAKING DIRECTORY \n");
	if(dir.indexLocation <= 0){
		printf("UNINTALIZED");
		dEntry = parsePath(".");
	} else {
		printf("INITALIZED");
		dEntry = parsePath(dir.pathName);
	}
	if(strcmp(dEntry.name, pathname) == 0){
		return -1;
	}
	DEntry* currentDir = malloc(dEntry.size);
	DEntry* tmp = loadDirectory(dEntry);
	currentDir = memcpy(currentDir,tmp,tmp[dir.indexLocation].size);
	free(tmp);
	printf("\n MAP information map Free Bit: [%d], map BitLocation [%d], map Index = [%d] \n ",	map.freeBits, map.bitLocation, map.index);
	int freeDEntry = freeDirectory(currentDir);
	if(freeDEntry == -1){
		printf("\n Failed To Make Directory\n ");
		return -1;
	}
	//Parent Location
	char* parentName = currentDir[0].name;
	int parentLocation = currentDir[0].returnBlock;
	printf("\n parentName: [%s]\n", parentName);
	printf("\n parentLocation: [%d]\n", parentLocation);
	//self Location
	int selfLocation = map.freeBits +1;
	map.freeBits = selfLocation;
	int size = currentDir[0].size;
	int blocksRead = currentDir[0].blockToRead;
	printf("\n selfLocation: [%d]\n", map.freeBits);
	printf("\n size: [%d]\n", size);
	printf("\n blocksRead: [%d]\n", blocksRead);

	// //Setting up directory to be found
	strcpy(currentDir[freeDEntry].name, pathname);
	currentDir[freeDEntry].freeState = 1;
	currentDir[freeDEntry].returnBlock = selfLocation;
	currentDir[freeDEntry].type = 1;
	currentDir[freeDEntry].blockToRead = blocksRead;
	currentDir[freeDEntry].size = size;

	// //Write the new Directory
	LBAwrite(currentDir, blocksRead, currentDir[0].returnBlock);

	//Setting up the directory
	LBAread(currentDir,blocksRead, currentDir[freeDEntry].returnBlock);

	//New Directory
	strcpy(currentDir[0].name, pathname);
	currentDir[0].returnBlock = selfLocation;
	currentDir[0].size = size;
	currentDir[0].type = 1;
	currentDir[0].freeState = 1;
	currentDir[0].blockToRead = blocksRead;
	//pointing to parent directory
	strcpy(currentDir[1].name, parentName);
	currentDir[1].returnBlock = parentLocation;
	currentDir[1].size = size;
	currentDir[1].type = 1;
	currentDir[1].freeState = 1;
	currentDir[1].blockToRead = blocksRead;
	LBAwrite(currentDir, blocksRead, selfLocation);
	//Reset the location to parent Directory
	LBAread(currentDir,blocksRead,currentDir[1].returnBlock);
	free(currentDir);
	return 0;
	
}


// 1 is dir
int fs_isDir(char * pathname) {
	int error = 0;
	if(parsePath(pathname).type != 1){
		error = -1;
	}
	return error;
}


//in mfs.h     2 = file, 0 = not file
int fs_isFile(char * pathname) {

	//not file
	int error = 0;
	if(parsePath(pathname).type != 2){
		error = -1;
	}
}

int fs_setcwd(char *pathname){
	int error = 0;
	parsePath(pathname);
	if(dir.indexLocation == -1){
		error = -1;
	}
	return error;
}

char* fs_getcwd(char *pathname, size_t size){
	if(strcasecmp(pathname, "") == 0){
		parsePath(".");
	}
	if(dir.indexLocation == -1){
		return NULL;
	}
	strncpy(pathname, dir.pathName,size);
	return dir.pathName;
}
int fs_rmdir(const char* pathname) {
    // This is the object of the current working directory
    DEntry dentry = parsePath(pathname);
    DEntry* directory = loadDirectory(dentry);
    // If parsepath couldn't find the directory.
    if(dir.indexLocation == -1){
        printf("Error: Not a directory.");
        return -1;
    } else {
        for(int i = 2; i < 58; i++){
            // Check if directory is empty
            if(directory[i].freeState != 0){
                printf("Error: Not an empty directory.");
                return -1;
            } 
        }
        dentry.freeState == 0;
    }
    free(directory);
    return 0;
}

int fs_delete(char* filename){
	DEntry file = parsePath(filename);
	if (dir.indexLocation == -1)
	{
		printf("Can't delete file does not exist!");
		return -1;
	}else
	{
		DEntry * dir = loadDirectory(file);
		file.freeState = 0;
		LBAwrite(dir, dir[0].blockToRead,dir[0].returnBlock); 
		free(dir);
		return 0;
	}
}

fdDir * fs_opendir(const char *pathname){
	char* path = malloc(strlen(pathname)+1);
	if (parsePath(path).returnBlock != 0 && fs_isDir(path) == 1)
	{
		fdDir *fd = malloc(sizeof(fdDir));
		fd->dirEntryPosition = 0;
		fd->dir = loadDirectory(parsePath(path));
		free(path);
		return fd;

	}
	else
	{
		free(path);
		return NULL;
	}
}

struct fs_diriteminfo *fs_readdir(fdDir *dirp){

	//loops through directory
	for (int i = dirp->dirEntryPosition; i < dirp->d_reclen; i++)
	{
		//basically checks to see if the entry is empty or not
		if(dirp->dirItem[i] != NULL)
		{
			//copy name and filetype
			strcpy(dirp->dirItem[i]->d_name, dirp->dir[i].name);
			dirp->dirItem[i]->fileType = dirp->dir[i].type;
			//returns the next item in the entry
			return dirp->dirItem[i+1];
		}
		
	}
	return NULL;
}

int fs_closedir(fdDir *dirp){
	for (int i = 0; i < dirp->d_reclen; i++)
	{
		//freeing up
		free(dirp->dirItem[i]);
	}
	free(dirp->dirItem);
	free(dirp);
}
