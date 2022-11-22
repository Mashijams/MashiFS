# MashiFS

MashiFS is a virtual file system written in C++ from scratch.

It has an Unix like file system implementation with Inodes and simple on-disk structures to handle directories and files.

### Compiling MashiFS

On project directory simple run **make** command on terminal.

It will compile all files for MashiFS and create an executable file named **MashiFS**

### Creating new disk

After successfully compiling MashiFS to create a new disk run command : 

`./MashiFS create <diskname> <TotalBlocks>`

where diskname is name you want to give to your disk and TotalBlocks is total number of disk blocks you want for you disk.

Each DiskBlock is of size 4096 bytes or 4KB.

To create a disk named Raghav with 1MB size Total Blocks required will be 256 and command would be :

`./MashiFS create Raghav 256`

Creating a new disk will mount disk simultaneously.

### Mounting an existing disk

After successfully compiling MashiFS to mount an existing disk in Project Directory run command :

`./MashiFS mount <diskname>`

For example to mount a disk named Raghav run command :

`./MashiFS mount Raghav`

### File System Commands

After successfull mounting of Disk with MashiFS a shell client named fssh will be activated.

Type **help** command in shell to see all supported commands for MashiFS.


