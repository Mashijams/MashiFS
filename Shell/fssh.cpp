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
status_t Mount(Disk &disk, FileSystem &fs, char *arg1);
status_t Create(Disk &disk, FileSystem &fs, char *arg1, char *arg2);
void Help();
void Shell();


// Main execution
int main (int argc, char* argv[]) {
	Disk	disk;
    MashiFS	fs;

	if (argc != 3 && argc != 4) {
		fprintf(stderr, "Usage:  <MashiFS> <Mount> <DiskName>\n\n");
		fprintf(stderr, "Usage:  <MashiFS> <Create> <DiskName> <BlockSize>\n\n");
    	return EXIT_FAILURE;
	}

	// If we are creating a new disk with MashiFS
	if (StringEquals(argv[1], "Create")) {
		status_t status = Create(disk, fs, argv[2], argv[3]);
		if (status == F_SUCCESS)
			return EXIT_SUCCESS;

		// return failure otherwise
		return EXIT_FAILURE;
	}

	// If we are mounting existing disk
	if (StringEquals(argv[1], "Mount")) {
		status_t status = Mount(disk, fs, argv[2]);
		if (status == F_SUCCESS) {
			// if mount is successful run our shell
			Shell();
			return EXIT_SUCCESS;
		}

		// return failure otherwise
		return EXIT_FAILURE;
	}

}


status_t
Create(Disk &disk, FileSystem &fs, char *arg1, char *arg2)
{
	status_t status = disk.Create(fs, arg1, arg2);

	if (status == F_SUCCESS) {
		printf("Disk %s created successfully, you can now mount it\n\n", arg1);
	}

	if (status == F_FAIL) {
		printf("Disk %s creation failed\n\n", arg1);
	}

	return status;
}


status_t
Mount(Disk &disk, FileSystem &fs, char *arg1)
{
	status_t status = disk.Mount(fs, arg1);

	if (status == F_SUCCESS) {
		printf("Disk %s mounted successfully\n\n", arg1);
	}

	if (status == F_FAIL) {
		printf("Mounting disk %s failed\n\n", arg1);
	}

	return status;
}