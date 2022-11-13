// Main FS Implementation

#include "FS.h"


#include <stdio.h>
#include <math.h>
#include <string.h>


FileSystem::FileSystem()
{
	buffer	=	new char[BLOCK_SIZE];
	map		=	new InodeMap;
}


FileSystem::~FileSystem()
{
	delete[] buffer;
	delete map;
}


status_t
FileSystem::Init(Disk& disk, size_t TotalBlocks)
{
	this->disk = disk;

	/*
		Zero block of disk contains
		Superblock
		Inode Bitmap
	*/
	status_t status = _CreateSuperBlock(TotalBlocks);
	if (status != F_SUCCESS)
		return status;

	// Create Inode BitMap
	for (int i = 0; i < 124; i++) {
		map.Map[i] = 0;
	}

	// Root dir Inode is initialized
	map.Map[0] = 1;

	// write first block structures to disk now
	size_t offset = 0;

	char* ptr;
	ptr = (char*)&sb;
	memcpy(buffer + offset, ptr, sizeof(SuperBlock));
	offset += sizeof(SuperBlock);

	ptr = (char*)&map;
	memcpy(buffer + offset, ptr, sizeof(InodeMap));
	offset += sizeof(InodeMap);

	// Write to disk at block no zero
	status = disk.Write(buffer, 0);
	if (status != F_SUCCESS)
		return status;

	/*
		First block for file system is reserved
		for Inodes.
		It will get filled accordingly in Create Inode
		function.
	*/


	/*
		From second onwards we have data block bitmap
		Its header is in zeroth block
	*/
	for (int i = 2; i < 2 + sb.TotalBitmapBlocks; i++) {
		uint8_t	direct[BITMAPS_PER_BLOCK];
		for (int j  = 0; j < BITMAPS_PER_BLOCK; j++) {
			direct[j] = 0;
		}

		if (i == 2) {
			// first and second block are reserved
			direct[0] = direct[1] = 1;

			// Bitmaps blocks are reserved as well
			for (j = 2; j < 2 + sb.TotalBitmapBlocks; j++) {
				direct[j] = 1;
			}
		}

		ptr = (char*)direct;
		memcpy(buffer, ptr, BLOCK_SIZE);

		// Write to disk at block no i
		status = disk.Write(buffer, i);
		if (status != F_SUCCESS)
			return status;
	}

	// We had succesfully initialised FS on disk
	return F_SUCCESS;
}


status_t
FileSystem::Mount(Disk& disk)
{
	this->disk = disk;

	status_t status = disk.Read(buffer, 0);
	if (status != F_SUCCESS)
		return status;

	// Initialise and check superblock
	sb = *((SuperBlock*)buffer);

	if (sb.Magic != SB_MAGIC) {
		fprintf(stderr, "Wrong magic number for superblock\n\n");
		return F_FAIL;
	}

	// Currently we are at root directory
	inum = sb.root;

	// We successfully mounted FS
	return F_SUCCESS;
}


status_t
FileSystem::_CreateSuperBlock(size_t TotalBlocks)
{
	sb.Magic				=	SB_MAGIC;
	sb.TotalBlocks			=	TotalBlocks;
	sb.Root					=	0;
	sb.TotalFreeBlocks		=	TotalBlocks - 2 - ceil((double)TotalBlocks / 4096);
	sb.TotalBitmapBlocks	=	ceil((double)TotalBlocks / 4096);

	return F_SUCCESS;
}


uint16_t
FileSystem::_SearchFreeInode()
{
	/*
		Iterate through Inode Bitmap
		return first free Inode number
	*/
	for (uint16_t i = 1; i < 124; i++) {
		if (map[i] == 0)
			return i;
	}

	// All Inodes are reserved return 0
	return 0;
}


uint32_t
FileSystem::_SearchFreeBlock()
{
	/*
		Iterate through block Bitmap
		return first free block number
	*/
	for (uint8_t i = 0; i < sb.TotalBitmapBlocks; i++) {
		char data[BLOCK_SIZE];
		disk.Read(data, 2 + i);
		uint8_t* direct;
		direct = (uint8_t*)data;
		for (int j = 0; j < BITMAPS_PER_BLOCK; j++) {
			if (direct[j] == 0)
				return i * BITMAPS_PER_BLOCK + j;
		}
	}

	// 0 if none of the blocks are free
	return 0;
}