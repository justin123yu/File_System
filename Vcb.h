typedef struct VCB {
// Size of the block
int blockSize;

// Location of the beginning position
int startPostion;

// Location of the root
int rootLocation;

// location of freeSpace
int freeBlockLocation;

//size of Directory in bytes
int sizeOfDirectory;

//size of Directory in bytes
int blocksToRead;

// Magic Number
int magicSignature;
} VCB;

extern VCB* vcbpointer;