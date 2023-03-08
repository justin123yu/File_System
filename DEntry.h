typedef struct DEntry{
// Name of the file
char name[20];

// File type/extension
// If its a directory = 1, if its any other make it 2.
int type;

// Location of the directory in the bitmap (blockLocation)
int returnBlock;

// Size of the file
int size;

// Usage
int freeState;

// blockSize
int blockSize;

//Blocks to read
int blockToRead;

} DEntry;
