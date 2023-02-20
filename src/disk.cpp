// Interface between disk and file system

#include "../include/disk.h"


#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>


Disk::Disk()
	:
	FileDescriptor(0)
{
}


Disk::~Disk()
{
	if (FileDescriptor > 0) {
		close(FileDescriptor);
		FileDescriptor = 0;
	}
}


status_t
Disk::Create(char *DiskName, int TotalBlocks)
{
	// create and mount disk image
	// set persmisions of image to the owner
	FileDescriptor = ::open(DiskName, O_RDWR|O_CREAT, 0600);

	// if disk isn't created
	if (FileDescriptor < 0) {
		fprintf(stderr, "Unable to create disk image %s\n\n", DiskName);
		return F_FAIL;
    }

	// Make disk for desired blocks size
	if (ftruncate(FileDescriptor, TotalBlocks*BLOCK_SIZE) < 0) {
		fprintf(stderr, "Unable to set size of disk image %s\n\n", DiskName);
		return F_FAIL;
	}

	// Disk created successfully
	return F_SUCCESS;
}


status_t
Disk::Mount(char *DiskName)
{
	// mount disk image
	// set persmisions of image to the owner
	FileDescriptor = ::open(DiskName, O_RDWR, 0600);

	// if disk doesn't exist
	if (FileDescriptor < 0) {
		fprintf(stderr, "Unable to open disk image %s\n\n", DiskName);
		fprintf(stderr, "%s image doesn't exist\n\n", DiskName);
		return F_FAIL;
    }

	// Disk mounted successfully
	return F_SUCCESS;
}


status_t
Disk::Read(char* data, uint32_t blocknum)
{
	// set offset of file descriptor pointer to required blocknum
	if (lseek(FileDescriptor, blocknum*BLOCK_SIZE, SEEK_SET) < 0) {
    	fprintf(stderr, "Unable to set offset pointer in disk image\n\n");
    	return F_FAIL;
    }

	// Read data from disk in data buffer
    if (::read(FileDescriptor, data, BLOCK_SIZE) != BLOCK_SIZE) {
    	fprintf(stderr, "Unable to read data from disk image\n\n");
    	return F_FAIL;
    }

	return F_SUCCESS;
}


status_t
Disk::Write(char* data, uint32_t blocknum)
{
	// set offset of file descriptor pointer to required blocknum
	if (lseek(FileDescriptor, blocknum*BLOCK_SIZE, SEEK_SET) < 0) {
    	fprintf(stderr, "Unable to set offset pointer in disk image\n\n");
    	return F_FAIL;
    }

	// Read data from disk in data buffer
    if (::write(FileDescriptor, data, BLOCK_SIZE) != BLOCK_SIZE) {
    	fprintf(stderr, "Unable to write data from disk image\n\n");
    	return F_FAIL;
    }

	return F_SUCCESS;
}