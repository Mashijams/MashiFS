// Format types for MashiFS
#ifndef TYPE_H
#define TYPE_H


// This enum will determine what is status of process
enum status_t {
	F_SUCCESS,
	F_FAIL,
};


// This enum will determine whether given Inode is for directory or file
enum File_Type {
	F_IS_DIR,
	F_IS_FILE,
};


#endif
