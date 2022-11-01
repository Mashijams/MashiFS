#ifndef DISK_H
#define DISK_H


#include <stdlib.h>
#include "MashiFS/types.h"

#define BLOCK_SIZE 4096


class Disk {
public:
						Disk();
						~Disk();
			status_t	Create(char *DiskName, int TotalBlocks);
            status_t	Mount(char *DiskName);
			status_t	Read(char* data, uint32_t blocknum);
			status_t	Write(char* data, uint32_t blocknum);
private:
			int			FileDescriptor; // File descriptor of disk image
};

#endif