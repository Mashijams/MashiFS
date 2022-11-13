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
		We need to first create root inode for FS
	*/

	status = _CreateDirInode(0, &fInode, "..");
	if (status != F_SUCCESS)
		return status;

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

	// Initialise and Test Inode
	status = _ReadInodeFromDisk(inum, &fInode);
	if (status != F_SUCCESS)
		return status;

	if (fInode.Magic != IN_MAGIC) {
		fprintf(stderr, "Wrong magic number for root Inode\n\n");
		return F_FAIL;
	}

	// We successfully mounted FS
	return F_SUCCESS;
}


status_t
FileSystem::CreateDir(char* name)
{
	status_t status;

	// First see if we have an inode available
	uint16_t inumber  = _SearchFreeInode();
	if (inumber == 0) {
		fprintf(stderr, "No free inode available\n\n");
		return F_FAIL;
	}

	uint16_t requiredSpace = 3 + strlen(name);

	// find free space from allocated directory blocks
	for (uint8_t i = 0; i < fInode.TotalDataBlocks; i++) {
		char data[BLOCK_SIZE];
		status = disk.Read(data, fInode.Direct[i]);
		if (status != F_SUCCESS)
			return status;
		DirectoryHeader* header = (DirectoryHeader*)data;

		// Found an existing block with free space
		if (header->FreeSpace >= requiredSpace) {
			// Found block now add entry
			DirectoryEntry entry;
			entry.Inumber = inumber;
			entry.namelen = strlen(name);
			memcpy(entry.name, name, entry.namelen);

			// write entry to data
			char* ptr;
			int offset = BLOCK_SIZE - header->FreeSpace;
			ptr = (char*)&entry;
			memcpy(data + offset, ptr, _SizeOfEntry(&entry));

			// update header freeSpace
			offset = 0;
			header->FreeSpace -= _SizeOfEntry(&entry);
			header->TotalEntries += 1;
			ptr = (char*)header;
			memcpy(data, ptr, _SizeOfDirectoryHeader(header));

			// write data to disk
			status = disk.write(data, fInode.Direct[i]);

			if (status != F_SUCCESS)
				return status;

			// Allocate inode for new directory
			Inode newInode;
			status = _CreateDirInode(inumber, &newInode, name);

			return status;
		}
	}

	// Now if none of the data blocks has enough space
	if (fInode.TotalDataBlocks >= 5) {
		fprintf(stderr, "No free data block available\n\n");
		return F_FAIL;
	}

	// Allocate new directory block
	fInode.Direct[fInode.TotalDataBlocks] = _SearchFreeBlock();
	if (fInode.Direct[fInode.TotalDataBlocks] == 0) {
		fprintf(stderr, "No free data block available\n\n");
		return F_FAIL;
	}

	status = _CreateDirectoryHeader(fInode.Direct[fInode.TotalDataBlocks], name);
	if (status != F_SUCCESS)
		return status;

	char data[BLOCK_SIZE];
	status = disk.Read(data, fInode.Direct[fInode.TotalDataBlocks]);
	if (status != F_SUCCESS)
		return status;
	DirectoryHeader* header = (DirectoryHeader*)data;

	if (header->FreeSpace >= requiredSpace) {
			// Found block now add entry
			DirectoryEntry entry;
			entry.Inumber = inumber;
			entry.namelen = strlen(name);
			memcpy(entry.name, name, entry.namelen);

			// write entry to data
			char* ptr;
			int offset = BLOCK_SIZE - header->FreeSpace;
			ptr = (char*)&entry;
			memcpy(data + offset, ptr, _SizeOfEntry(&entry));

			// update header freeSpace
			offset = 0;
			header->FreeSpace -= _SizeOfEntry(&entry);
			header->TotalEntries += 1;
			ptr = (char*)header;
			memcpy(data, ptr, _SizeOfDirectoryHeader(header));

			// write data to disk
			status = disk.write(data, fInode.Direct[fInode.TotalDataBlocks]);

			if (status != F_SUCCESS)
				return status;

			// Allocate inode for new directory
			Inode newInode;
			status = _CreateDirInode(inumber, &newInode, name);

			return status;
	}
	else {
		fprintf(stderr, "Name is too big\n\n");
		return F_FAIL;
	}

	// update existing Inode
	fInode.TotalDataBlocks += 1;
	status = _WriteInodeToDisk(inum, &fInode);
	if (status != F_SUCCESS)
		return status;

	// Directory successfully created
	return F_SUCCESS;
}


status_t
FileSystem::ChangeDir(char* name, char* dirName)
{
	// First search for entry
	status_t status = _SearchForEntry(name);
	if (status != F_SUCCESS)
		return status;

	// Read this directory Inode and Test it
	status = _ReadInodeFromDisk(&inum, &fInode);

	if (fInode.Magic != IN_MAGIC) {
		fprintf(stderr, "Wrong magic number for root Inode\n\n");
		return F_FAIL;
	}

	// Get Directory name from Disk
	char data[BLOCK_SIZE];
	status = disk.Read(data, fInode.Direct[0]);
	if (status != F_SUCCESS)
		return status;

	DirectoryHeader* header = (DirectoryHeader*)data;
	// Not root directory
	if (strcmp(header->name, "..") == 1) {
		memcpy(dirName, header->name, header->namelen);
	}
	else { // It is root directory
		dirName[0] = '\0';
	}

	// Successfully changed directory
	return F_SUCCESS;
}


status_t
FileSystem::ListAllEntries()
{
	status_t status;

	// Traverse through all data blocks
	for (uint8_t i = 0; i < fInode.TotalDataBlocks; i++) {
		char data[BLOCK_SIZE];
		status = disk.Read(data, fInode.Direct[i]);
		if (status != F_SUCCESS)
			return status;

		DirectoryHeader* header = (DirectoryHeader*)data;
		DirectoryEntry* entry;
		int offset = _SizeOfDirectoryHeader(header);

		//Traverse through all entries
		for (int j = 0; j < header->TotalEntries; j++) {
			entry = (DirectoryEntry*)(data + offset);
			printf("%s\n", entry->name);
			offset += _SizeOfEntry(entry);
		}
	}

	return F_SUCCESS;
}


size_t
FileSystem::Size()
{
	return sb.TotalBlocks;
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
		if (map[i] == 0) {
			map[i] = 1;
			char data[BLOCK_SIZE];
			disk.Read(data, 0);
			char* ptr;
			ptr = (char*)map;
			memcpy(data + sizeof(SuperBlock), ptr, sizeof(InodeMap));
			disk.Write(data, 0);
			return i;
		}
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
			if (direct[j] == 0) {
				direct[j] = 1;
				char* ptr;
				ptr = (char*)direct;
				memcpy(data, ptr, BLOCK_SIZE);
				disk.Write(data, 2 + i);
				return i * BITMAPS_PER_BLOCK + j;
			}
		}
	}

	// 0 if none of the blocks are free
	return 0;
}


status_t
FileSystem::_WriteInodeToDisk(uint16_t* inumber, Inode* inode)
{
	char data[BLOCK_SIZE];

	status_t status = disk.Read(data, 1);
	if(status != F_SUCCESS)
		return status;

	char* ptr;
	ptr = (char*)inode;
	memcpy(data + inumber * sizeof(Inode), ptr, sizeof(Inode));

	status = disk.Write(data, 1);
	if (status != F_SUCCESS)
		return status;

	return F_SUCCESS;
}


status_t
FileSystem::_ReadInodeFromDisk(uint16_t* inumber, Inode* inode)
{
	char data[BLOCK_SIZE];

	status_t status = disk.Read(data, 1);
	if(status != F_SUCCESS)
		return status;

	inode = (Inode*)(data + inumber * sizeof(Inode));

	return F_SUCCESS;
}


status_t
FileSystem::_CreateDirectoryHeader(uint32_t* blocknum, char* name)
{
	DirectoryHeader header;

	header.Magic = DIR_MAGIC;
	header.TotalEntries = 1;
	header.namelen = strlen(name);
	memcpy(header.name, name, header.namelen);

	// Create parent entry
	DirectoryEntry entry;
	entry.Inumber = inum;
	entry.namelen = 2;
	memcpy(entry.name, "..", entry.namelen);

	header.FreeSpace = BLOCK_SIZE - _SizeOfDirectoryHeader(&header) - _SizeOfEntry(&entry);

	char data[BLOCK_SIZE];

	char* ptr;
	ptr = (char*)&header;
	memcpy(data, ptr, _SizeOfDirectoryHeader(&header));

	ptr = (char*)&entry;
	memcpy(data + _SizeOfDirectoryHeader(&header), ptr, _SizeOfEntry(&entry));

	status_t status = disk.Write(data, blocknum);
	if (status != F_SUCCESS)
		return status;

	return F_SUCCESS;
}


uint16_t
FileSystem::_SizeOfDirectoryHeader(DirectoryHeader* header)
{
	return 7 + header->namelen;
}


uint16_t
FileSystem::_SizeOfEntry(DirectoryEntry* entry)
{
	return 3 + entry->namelen;
}


status_t
FileSystem::_CreateDirInode(uint16_t inumber, Inode* inode, char* name)
{
	inode->Magic			=	IN_MAGIC;
	inode->Size				=	0;
	inode->Type				=	F_IS_DIR;
	inode->TotalDataBlocks	=	1;

	for (uint8_t i = 1; i < 5; i++)
		inode->Direct[i] = 0;

	inode->Indirect = 0;

	inode->Direct[0] = _SearchFreeBlock();
	if (inode->Direct[0] == 0) {
		fprintf(stderr, "No free space block available\n\n");
		return F_FAIL;
	}

	// Initialise directory header for this block
	status = _CreateDirectoryHeader(inode->Direct[0], name);
	if (status != F_SUCCESS)
		return status;

	// Now write Inode to disk
	char data[BLOCK_SIZE];
	status_t status = disk.Read(data, 1);
	if (status != F_SUCCESS)
		return status;

	char* ptr;
	ptr = (char*)inode;
	memcpy(data + inumber * sizeof(Inode), ptr, sizeof(Inode));

	status = disk.Write(data, 1);
	if (status != F_SUCCESS)
		return status;

	// Inode written to disk successfully
	return F_SUCCESS;
}


status_t
FileSystem::_SearchForEntry(char* name)
{
	status_t status;

	// Traverse through all data blocks
	for (uint8_t i = 0; i < fInode.TotalDataBlocks; i++) {
		char data[BLOCK_SIZE];
		status = disk.Read(data, fInode.Direct[i]);
		if (status != F_SUCCESS)
			return status;

		DirectoryHeader* header = (DirectoryHeader*)data;
		DirectoryEntry* entry;
		int offset = _SizeOfDirectoryHeader(header);

		//Traverse through all entries
		for (int j = 0; j < header->TotalEntries; j++) {
			entry = (DirectoryEntry*)(data + offset);
			if (strcmp(name, entry->name) == 0) {
				inum = entry->Inumber;
				return F_SUCCESS;
			}
			offset += _SizeOfEntry(entry);
		}
	}

	return F_ENTRY_NOT_EXIST
}