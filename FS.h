#ifndef MASHI_FS_H
#define MASHI_FS_H


#include "disk.h"
#include "types.h"
#include <stdint.h>


#define SB_MAGIC 0x4C5
#define IN_MAGIC 0xCEF
#define	DIR_MAGIC 0xCEE
#define POINTERS_PER_INODE 5
#define POINTERS_PER_BLOCK 1024
#define BITMAPS_PER_BLOCK 4096
#define MAX_DATA_BLOCKS 5
#define TOTAL_INODE 124


// This superblock is at first disk block
struct SuperBlock {
		uint16_t		Magic;				// Magic number of superblock
		size_t			TotalBlocks;		// total number of blocks in file system
		uint16_t		Root;				// root inode number of file system (typically 0)
		uint32_t		TotalFreeBlocks;	// Total Free Blocks available
		uint8_t			TotalBitmapBlocks;	// Total Blocks associated for block bitmap
};


// Inode Bitmap starts at second disk block
struct Inode {
		uint16_t		Magic;						// Magic number of Inode
		uint32_t		Size;						// size of file
		uint8_t			Type;						// Is it file or directory
		uint16_t		TotalDataBlocks;			// total data blocks associated with this inode
		uint32_t		Direct[POINTERS_PER_INODE];	// Direct pointers for data
		uint32_t		Indirect;					// Indirect pointers for data
};


// Data Header for Directory block
struct DirectoryHeader {
		uint16_t		Magic;			// Magic number for header
		uint16_t		FreeSpace;		// Total Freespace in this directory block
		uint16_t		TotalEntries;	// Total number of entries in this directory
		uint8_t			namelen;		// Directory name length
		char			name[]			// Directory name
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


// Bitmpas to track free Inode and free FS Blocks
// This structure is after superblock in first FS Block
struct InodeMap {
		uint8_t			Map[124];
};


class FileSystem {
public:
							FileSystem();
							~FileSystem();
			status_t		Init(Disk& disk, size_t TotalBlocks);
			status_t		Mount(Disk& disk);
			status_t		CreateDir(char* name);
			status_t		ChangeDir(char* name);
			status_t		ListAllEntries();
			size_t			Size();

private:
			status_t		_CreateSuperBlock(size_t TotalBlocks);
			status_t		_WriteInodeToDisk(uint16_t* inumber, Inode* inode);
			status_t		_ReadInodeFromDisk(uint16_t* inumber, Inode* inode);
			status_t		_CreateDirectoryHeader(uint32_t* blocknum, char* name);
			status_t		_CreateDirInode(uint16_t inumber, Inode* inode, char* name);
			status_t		_SearchForEntry(char* name);
			uint16_t		_SearchFreeInode();
			uint32_t		_SearchFreeBlock();
			uint16_t		_SizeOfDirectoryHeader(DirectoryHeader* header);
			uint16_t		_SizeOfEntry(DirectoryEntry* entry);

			Disk			disk;
			SuperBlock		sb;
			char*			buffer;
			uint16_t		inum;
			InodeMap*		map;
			Inode			fInode;
};

#endif