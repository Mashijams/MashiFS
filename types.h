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


// This enum will store shell command hash
enum Command {
	F_HELP,
	F_CD,
	F_CAT,
	F_TOUCH,
	F_MKDIR,
	F_RM,
	F_LS,
	F_COPY,
	F_MOVE,
	F_QUIT,
	F_EXIT,
	F_UNKNOWN,
};

#endif
