// Shell to interact with MashiFS


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// file system headers
#include "disk.h"
#include "FS.h"
#include "types.h"


// Macros
#define	StringEquals(a, b)	(strcmp((a), (b)) == 0)


// Command Shell functions
status_t	Mount(Disk &disk, FileSystem &fs, char *DiskName);
status_t	Create(Disk &disk, FileSystem &fs, char *DiskName, size_t TotalBlocks);
void		Shell(FileSystem &fs);


// Main execution
int main (int argc, char* argv[]) {
	Disk	disk;
	FileSystem	fs;

	if (argc != 4 && argc != 3) {
		fprintf(stderr, "Usage:  <Mount> <DiskName>\n\n");
		fprintf(stderr, "Usage:  <Create> <DiskName> <TotalBlocks>\n\n");
    	return EXIT_FAILURE;
	}

	// If we are creating a new disk with MashiFS
	if (StringEquals(argv[1], "Create")) {
		status_t status = Create(disk, fs, argv[2], atoi(argv[3]));
		if (status == F_SUCCESS) {
			// if creation and mount is successful run our shell
			Shell(fs);
			return EXIT_SUCCESS;
		}

		// return failure otherwise
		return EXIT_FAILURE;
	}

	// If we are mounting existing disk
	if (StringEquals(argv[1], "Mount")) {
		status_t status = Mount(disk, fs, argv[2]);
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
	status_t status = disk.Create(DiskName, TotalBlocks);

	if (status == F_SUCCESS) {
		printf("Disk %s created successfully\n\n", DiskName);
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
	status_t status = disk.Mount(DiskName);

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