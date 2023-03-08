

typedef struct TheFreeSpaceMap{
	//Bit Location
	int bitLocation;

	//Byte Location
	int index;


	//Free bit
	int freeBits;
}TheFreeSpaceMap;

//Global Free Space Map
//extern exist but not inialized here.
extern TheFreeSpaceMap map;

extern unsigned char* bitmap;

int bitCounter(unsigned char target);