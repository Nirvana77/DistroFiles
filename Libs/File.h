#ifndef File_h__
#define File_h__

#include <stdio.h>
#include "Hash/md5.h"

typedef enum
{
    File_Mode_Read = 0,
    File_Mode_ReadBinary = 1,
    File_Mode_Write = 2,
    File_Mode_WriteBinary = 3,
    File_Mode_Apend = 4,
    File_Mode_ApendCreate = 5,
    File_Mode_ReadWrite = 6,
    File_Mode_ReadWriteBinary = 7,
    File_Mode_ReadWriteCreate = 8,
    File_Mode_ReadWriteCreateBinary = 9,
    File_Mode_ReadApend = 10,
    File_Mode_ReadApendBinary = 11
} File_Mode;

const char* File_Mode_String[] = {
    "r",
    "rb",
    "w",
    "wb",
    "a",
    "ab",
    "r+",
    "rb+",
    "w+",
    "wb+",
    "a+",
    "ab+",
};

int File_Open(const char* _FilePath, File_Mode _Mode, FILE** _FilePtr);
void File_SetPosition(FILE* _File, int _Position);
unsigned int File_GetSize(FILE* _File);
void File_Close(FILE* _File);

int File_Hash(FILE* _File, unsigned char _Result[16]);

int File_GetHash(const char* _Path, unsigned char _Result[16])
{
    FILE* f = NULL;
    File_Open(_Path, File_Mode_ReadBinary, &f);
    if(f == NULL)
        return -1;

    if(File_Hash(f, _Result) != 0)
    {
        File_Close(f);
        return -2;
    }

    File_Close(f);
    return 0;
}

int File_Read(FILE* _File, unsigned char* _Buffer, int _Size);
int File_ReadAll(FILE* _File, unsigned char* _Buffer, int _BufferSize);

int File_WriteAll(FILE* _File, const unsigned char* _Data, int _DataSize);

int File_Copy(const char* _Source, const char* _Destination);
int File_Move(const char* _Source, const char* _Destination);
int File_Remove(const char* _Source);


#endif // File_h__