#ifndef MASHI_FS_H
#define MASHI_FS_H


#include "disk/disk.h"
#include "MashiFS/types.h"
#include <stdint.h>


#define SB_MAGIC 0x4C5
#define IN_MAGIC 0xCEF
#define	DIR_MAGIC 0xCEE
#define POINTERS_PER_INODE 5
#define POINTERS_PER_BLOCK 1024
#define TOTAL_INODE 132


// This superblock is at first disk block
struct SuperBlock {
		uint16_t		Magic;				// Magic number of superblock
		size_t			TotalBlocks;		// total number of blocks in file system
		uint16_t		Root;				// root inode number of file system (typically 0)
		uint32_t		TotalFreeBlocks;	// Total Free Blocks available
};


// Inode Bitmap starts at second disk block
struct Inode {
		uint16_t		Magic;						// Magic number of Inode
		uint32_t		Size;						// size of file
		uint8_t			Type;						// Is it file or directory
		uint32_t		Direct[POINTERS_PER_INODE];	// Direct pointers for data
		uint32_t		Indirect;					// Indirect pointers for data
};


// Data Header for Directory block
struct DirectoryHeader {
		uint16_t		Magic;			// Magic number for header
		uint16_t		TotalEntries;	// Total number of entries in this directory
};


// Indirect ptr block for direct pointers
struct IndirectBlock {
		uint32_t		Direct[POINTERS_PER_BLOCK];
};


// Entries format in directory block
struct DirectoryEntry {
		uint16_t		Inumber;
		uint8_t			namelen;
		char			name[];
};


class FileSystem {
public:
			status_t		Init(Disk& disk, size_t TotalBlocks);
			status_t		Mount(Disk& disk);
			size_t			Size();
private:
			Disk			disk;
			SuperBlock		sb;
};

#endif