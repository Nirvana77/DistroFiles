#ifndef File_h__
#define File_h__

#include <stdio.h>
#include "Hash/md5.h"

int File_Open(const char* _FilePath, const char* _Mode, FILE** _FilePtr);
void File_SetPosition(FILE* _File, int _Position);
unsigned int File_GetSize(FILE* _File);
void File_Close(FILE* _File);

int File_Hash(FILE* _File, unsigned char _Result[16]);

int File_GetHash(const char* _Path, unsigned char _Result[16])
{
    FILE* f = NULL;
    File_Open(_Path, "rb", &f);
    if(f == NULL)
        return -1;

    File_Hash(f, _Result);

    File_Close(f);
}

int File_Read(FILE* _File, unsigned char* _Buffer, int _Size);
int File_ReadAll(FILE* _File, unsigned char* _Buffer, int _BufferSize);

int File_WriteAll(FILE* _File, const unsigned char* _Data, int _DataSize);

int File_Copy(const char* _Source, const char* _Destination);
int File_Move(const char* _Source, const char* _Destination);
int File_Remove(const char* _Source);


#endif // File_h__