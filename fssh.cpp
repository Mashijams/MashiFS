// Shell to interact with MashiFS


#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>


// file system headers
#include "disk.h"
#include "FS.h"
#include "types.h"


// Macros
#define	StringEquals(a, b)	(strcmp((a), (b)) == 0)

// Map for storing shell command equivalent code
std::map<std::string, Command> command_hash;

// Command Shell functions
status_t	Mount(Disk &disk, FileSystem &fs, char *DiskName);
status_t	Create(Disk &disk, FileSystem &fs, char *DiskName, size_t TotalBlocks);
status_t	Shell(FileSystem &fs);
status_t	Help();
status_t	Cd(FileSystem& fs, char* arg1);
status_t	Cat(FileSystem& fs, char* arg1);
status_t	Touch(FileSystem& fs, char* arg1);
status_t	Mkdir(FileSystem& fs, char* arg1);
status_t	Rm(FileSystem& fs, char* arg1);
status_t	Ls(FileSystem& fs);
status_t	Copy(FileSystem& fs, char* arg1, char* arg2);
status_t	Move(FileSystem& fs, char* arg1, char* arg2);


void
_setup_command_hash_map()
{
	command_hash["help"]	=	F_HELP;
	command_hash["cd"]		=	F_CD;
	command_hash["cat"]		=	F_CAT;
	command_hash["touch"]	=	F_TOUCH;
	command_hash["mkdir"]	=	F_MKDIR;
	command_hash["rm"]		=	F_RM;
	command_hash["ls"]		=	F_LS;
	command_hash["copy"]	=	F_COPY;
	command_hash["move"]	=	F_MOVE;
	command_hash["quit"]	=	F_QUIT;
	command_hash["exit"]	=	F_EXIT;
}


// Main execution
int main (int argc, char* argv[]) {
	Disk	disk;
	FileSystem	fs;

	if (argc != 4 && argc != 3) {
		fprintf(stderr, "Usage:  <mount> <DiskName>\n\n");
		fprintf(stderr, "Usage:  <create> <DiskName> <TotalBlocks>\n\n");
    	return EXIT_FAILURE;
	}

	// setup command_hash map
	_setup_command_hash_map();

	if (StringEquals(argv[1], "create")) {
		// If we are creating a new disk with MashiFS
		status_t status = Create(disk, fs, argv[2], atoi(argv[3]));
		if (status != F_SUCCESS)
			return EXIT_FAILURE;

		status = Shell(fs);
		if (status != F_SUCCESS)
			return EXIT_FAILURE;

	} else if (StringEquals(argv[1], "mount")) {
		// If we are mounting existing disk
		status_t status = Mount(disk, fs, argv[2]);
		if (status != F_SUCCESS)
			return EXIT_FAILURE;

		status = Shell(fs);
		if (status != F_SUCCESS)
			return EXIT_FAILURE;

	} else {
		fprintf(stderr, "Usage:  <mount> <DiskName>\n\n");
		fprintf(stderr, "Usage:  <create> <DiskName> <TotalBlocks>\n\n");
	}

	return EXIT_SUCCESS;
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


status_t
Shell(FileSystem &fs)
{
	char name[100] = "";

	while(true) {

		fprintf(stderr, "fssh>%s> ", name);
    	fflush(stderr);

		char line[BUFSIZ], cmd[BUFSIZ], arg1[BUFSIZ], arg2[BUFSIZ];

		// read the input from user in line
		if (fgets(line, BUFSIZ, stdin) == NULL) {
    	    break;
    	}

		// set command strings from line to respective variables
    	int args = sscanf(line, "%s %s %s", cmd, arg1, arg2);
    	if (args == 0) {
			// if no command is entered
    	    continue;
		}

		status_t status;

		Command c;

		if (command_hash.find(cmd) != command_hash.end())
			c = command_hash[cmd];
		else
			c = F_UNKNOWN;

		switch(c) {

			case F_HELP:
					status = Help();
					break;

			case F_CD:
					status = Cd(fs, arg1, name);
					break;

			case F_CAT:
					status = Cat(fs, arg1);
					break;

			case F_TOUCH:
					status = Touch(fs, arg1);
					break;

			case F_MKDIR:
					status = Mkdir(fs, arg1);
					break;

			case F_RM:
					status = Rm(fs, arg1);
					break;

			case F_LS:
					status = Ls(fs);
					break;

			case F_COPY:
					status = Copy(fs, arg1, arg2);
					break;

			case F_MOVE:
					status = Move(fs, arg1, arg2);
					break;

			case F_QUIT:
					printf("Quiting FS shell\n");
					return F_SUCCESS;

			case F_EXIT:
					printf("Quiting FS shell\n");
					return F_SUCCESS;

			case F_UNKNOWN:
					printf("Unknown command: %s\n", line);
	    			printf("Type 'help' for a list of commands.\n\n");
					status = F_SUCCESS;
					break;

			default:
					break;
		}

		if (status != F_SUCCESS)
			return F_FAIL;
	}

	return F_SUCCESS;
}


status_t
Help()
{
	printf("Commands are:\n\n");
    printf("help                (To list all commands supported)\n\n");
    printf("cd <dir>            (change directory to <dir>)\n\n");
    printf("cat <file>          (see <file> contents)\n\n");
	printf("touch <file>        (create new <file>)\n\n");
    printf("mkdir <dir>         (create new directory <dir>)\n\n");
    printf("rm <file/dir>       (remove directory or file)\n\n");
    printf("ls                  (list contents inside current directory)\n\n");
    printf("copy <path>         (copy file from path to current directory)\n\n");
    printf("move <file1> <path> (move file1 from directory to path)\n\n");
    printf("quit                (quit this shell)\n\n");
    printf("exit                (exit this shell)\n\n");
	return F_SUCCESS;
}


status_t
Cd(FileSystem& fs, char* arg1, char* name)
{
	status_t status = fs.ChangeDir(arg1, name);
	if (status == F_ENTRY_NOT_EXIST) {
		printf("Directory does not exist\n\n");
		return F_SUCCESS;
	}
	return status;
}


status_t
Cat(FileSystem& fs, char* arg1)
{
	// write here
	return F_SUCCESS;
}


status_t
Touch(FileSystem& fs, char* arg1)
{
	return F_SUCCESS;
}


status_t
Mkdir(FileSystem& fs, char* arg1)
{
	status_t status = fs.CreateDir(arg1);
	return status;
}


status_t
Rm(FileSystem& fs, char* arg1)
{
	// write here
	return F_SUCCESS;
}


status_t
Ls(FileSystem& fs)
{
	status_t status = fs.ListAllEntries();
	return status;
}


status_t
Copy(FileSystem& fs, char* arg1, char* arg2)
{
	// write here
	return F_SUCCESS;
}


status_t
Move(FileSystem& fs, char* arg1, char* arg2)
{
	// write here
	return F_SUCCESS;
}