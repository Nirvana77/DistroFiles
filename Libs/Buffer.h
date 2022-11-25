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
	Bool m_Dynamic;

	unsigned char* m_Ptr;
	unsigned char* m_ReadPtr;
	unsigned char* m_WritePtr;
	int m_ExtentionSize;
	int m_Size;

	int m_BytesLeft;

};

int Buffer_InitializePtr(Bool _IsDynamic, int _ExtentionSize, Buffer** _BufferPtr);
int Buffer_Initialize(Buffer* _Buffer, Bool _IsDynamic, int _ExtentionSize);

void Buffer_Clear(Buffer* _Buffer);

int Buffer_ExtendBy(Buffer* _Buffer, int _Size);
static inline int Buffer_Extend(Buffer* _Buffer)
{
	return Buffer_ExtendBy(_Buffer, _Buffer->m_ExtentionSize);
}

int Buffer_ReadUInt64(Buffer* _Buffer, UInt64* _Value);
int Buffer_ReadUInt32(Buffer* _Buffer, UInt32* _Value);
int Buffer_ReadUInt16(Buffer* _Buffer, UInt16* _Value);
int Buffer_ReadUInt8(Buffer* _Buffer, UInt8* _Value);

int Buffer_ReadBuffer(Buffer* _Buffer, unsigned char* _Ptr, int _Size);

int Buffer_WriteUInt64(Buffer* _Buffer, UInt64 _Value);
int Buffer_WriteUInt32(Buffer* _Buffer, UInt32 _Value);
int Buffer_WriteUInt16(Buffer* _Buffer, UInt16 _Value);
int Buffer_WriteUInt8(Buffer* _Buffer, UInt8 _Value);

int Buffer_WriteBuffer(Buffer* _Buffer, unsigned char* _Ptr, int _Size);
int Buffer_ReadFromFile(Buffer* _Buffer, FILE* _File);

int Buffer_Copy(Buffer* _Des, Buffer* _Src, int _Size);

static inline void Buffer_ResetReadPtr(Buffer* _Buffer)
{
	_Buffer->m_ReadPtr = _Buffer->m_Ptr;
	_Buffer->m_BytesLeft = abs(_Buffer->m_WritePtr - _Buffer->m_ReadPtr);
}
static inline void Buffer_ResetWritePtr(Buffer* _Buffer)
{
	_Buffer->m_WritePtr = _Buffer->m_Ptr;
	_Buffer->m_BytesLeft = abs(_Buffer->m_WritePtr - _Buffer->m_ReadPtr);
}
static inline void Buffer_Reset(Buffer* _Buffer)
{
	Buffer_ResetReadPtr(_Buffer);
	Buffer_ResetWritePtr(_Buffer);
}
static inline int Buffer_DeepCopy(Buffer* _Des, Buffer* _Src, int _Size)
{
	int written = Buffer_Copy(_Des, _Src, _Size);
	
	_Src->m_ReadPtr += written;
	_Src->m_BytesLeft -= written;

	return written;
}

void Buffer_Dispose(Buffer* _Buffer);

#endif // Buffer_h__
