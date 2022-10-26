#ifndef Folder_h__
#define Folder_h__

#include <errno.h>
#include "tinydir.h"
#include "File.h"

int Folder_Create(const char* _Path);
int Folder_Hash(const char* _Path, unsigned char _Result[16]);

static inline Bool Folder_IsEmpty(const char* _Path)
{
	tinydir_dir dir;
	if(tinydir_open(&dir, _Path) != 0)
		return True;
	
	Bool isEmpty = True;
	while (dir.has_next || isEmpty != True)
	{
		tinydir_file file;
		tinydir_readfile(&dir, &file);
		if(strcmp(file.name, ".") != 0 && strcmp(file.name, "..") != 0)
			isEmpty = False;
		
		tinydir_next(&dir);
	}

	return isEmpty;
}

#endif // Folder_h__