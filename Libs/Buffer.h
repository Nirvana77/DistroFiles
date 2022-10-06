#ifndef Buffer_h__
#define Buffer_h__

struct T_Buffer;
typedef struct T_Buffer Buffer;

#include "Types.h"
#include "Memory.h"

#ifndef TCPBufferSize
    #define TCPBufferSize 256
#endif

struct T_Buffer
{
    Bool m_Allocated;

    char* m_Ptr;
    char* m_ReadPtr;
    char* m_WritePtr;
    int m_Size;

};

int Buffer_InitializePtr(int _Size, Buffer** _BufferPtr);
int Buffer_Initialize(Buffer* _Buffer, int _Size);

int Buffer_ReadUInt64(Buffer* _Buffer, UInt64* _Value);
int Buffer_ReadUInt32(Buffer* _Buffer, UInt32* _Value);
int Buffer_ReadUInt16(Buffer* _Buffer, UInt16* _Value);
int Buffer_ReadUInt8(Buffer* _Buffer, UInt8* _Value);

int Buffer_WriteUInt64(Buffer* _Buffer, UInt64 _Value);
int Buffer_WriteUInt32(Buffer* _Buffer, UInt32 _Value);
int Buffer_WriteUInt16(Buffer* _Buffer, UInt16 _Value);
int Buffer_WriteUInt8(Buffer* _Buffer, UInt8 _Value);

void Buffer_Dispose(Buffer* _Buffer);

#endif // Buffer_h__