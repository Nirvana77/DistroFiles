#ifndef Folder_h__
#define Folder_h__

#include <errno.h>
#include "tinydir.h"
#include "File.h"

int Folder_Create(const char* _Path);
int Folder_Hash(const char* _Path, unsigned char _Result[16]);

#endif // Folder_h__