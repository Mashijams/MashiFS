// Shell to interact with MashiFS


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// file system headers
#include "disk/disk.h"
#include "MashiFS/MashiFS.h"
#include "MashiFS/types.h"


// Macros
#define	StringEquals(a, b)	(strcmp((a), (b)) == 0)


// Command Shell functions
status_t	Mount(Disk &disk, FileSystem &fs, char *DiskName);
status_t	Create(Disk &disk, FileSystem &fs, char *DiskName, int TotalBlocks);
void		Shell(FileSystem &fs);


// Main execution
int main (int argc, char* argv[]) {
	Disk	disk;
	MashiFS	fs;

	if (argc != 2 && argc != 3) {
		fprintf(stderr, "Usage:  <MashiFS> <Mount> <DiskName>\n\n");
		fprintf(stderr, "Usage:  <MashiFS> <Create> <DiskName> <TotalBlocks>\n\n");
    	return EXIT_FAILURE;
	}

	// If we are creating a new disk with MashiFS
	if (StringEquals(argv[0], "Create")) {
		status_t status = Create(disk, fs, argv[1], stoi(argv[2]));
		if (status == F_SUCCESS) {
			// if creation and mount is successful run our shell
			Shell(fs);
			return EXIT_SUCCESS;
		}

		// return failure otherwise
		return EXIT_FAILURE;
	}

	// If we are mounting existing disk
	if (StringEquals(argv[0], "Mount")) {
		status_t status = Mount(disk, fs, argv[1]);
		if (status == F_SUCCESS) {
			// if mount is successful run our shell
			Shell(fs);
			return EXIT_SUCCESS;
		}

		// return failure otherwise
		return EXIT_FAILURE;
	}

}


status_t
Create(Disk &disk, FileSystem &fs, char *DiskName, size_t TotalBlocks)
{
	status_t status = disk.Create(fs, DiskName, TotalBlocks);

	if (status == F_SUCCESS) {
		printf("Disk %s created successfully and mounted with MashiFS\n\n", DiskName);
	}

	if (status == F_FAIL) {
		printf("Disk %s creation failed\n\n", DiskName);
	}

	// Initialise MashiFS on newly created disk
	status = fs.Init(disk, TotalBlocks);

	return status;
}


status_t
Mount(Disk &disk, FileSystem &fs, char *DiskName)
{
	status_t status = disk.Mount(fs, DiskName);

	if (status == F_SUCCESS) {
		printf("Disk %s mounted successfully\n\n", DiskName);
	}

	if (status == F_FAIL) {
		printf("Mounting disk %s failed\n\n", DiskName);
	}

	// Mount FS from this disk
	status = fs.Mount(disk);

	return status;
}


void
Shell(FileSystem &fs)
{
	printf("Currently not implemented\n\n");
}