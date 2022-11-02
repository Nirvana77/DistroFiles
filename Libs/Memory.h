#ifndef Memory_h__
#define Memory_h__

#include "Types.h"

int Memory_UInt64ToBuffer(UInt64* _Src, unsigned char* _Pointer);
int Memory_UInt32ToBuffer(UInt32* _Src, unsigned char* _Pointer);
int Memory_UInt16ToBuffer(UInt16* _Src, unsigned char* _Pointer);
int Memory_UInt8ToBuffer(UInt8* _Src, unsigned char* _Pointer);

int Memory_ParseUInt64(const void* _Pointer, UInt64* _Dest);
int Memory_ParseUInt32(const void* _Pointer, UInt32* _Dest);
int Memory_ParseUInt16(const void* _Pointer, UInt16* _Dest);
int Memory_ParseUInt8(const void* _Pointer, UInt8* _Dest);

static inline int Memory_Copy(const void* _Des, const void* _Src, int _Size)
{
    int write = 0;
    for (int i = 0; i < _Size; i++)
        write += Memory_ParseUInt8(_Src + i, (UInt8*) (_Des + i));
    
    return write;
}

#endif // Memory_h__