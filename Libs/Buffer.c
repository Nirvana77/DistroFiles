#include "Buffer.h"

int Buffer_Extend(Buffer* _Buffer);



int Buffer_InitializePtr(Bool _IsDynamic, int _ExtentionSize, Buffer** _BufferPtr)
{
	Buffer* _Buffer = (Buffer*)Allocator_Malloc(sizeof(Buffer));
	if(_Buffer == NULL)
		return -1;
	
	int success = Buffer_Initialize(_Buffer, _IsDynamic, _ExtentionSize);
	if(success != 0)
	{
		Allocator_Free(_Buffer);
		return success;
	}
	
	_Buffer->m_Allocated = True;
	
	*(_BufferPtr) = _Buffer;
	return 0;
}

int Buffer_Initialize(Buffer* _Buffer, Bool _IsDynamic, int _ExtentionSize)
{
	_Buffer->m_Allocated = False;

	_Buffer->m_Ptr = (unsigned char*)Allocator_Malloc(sizeof(unsigned char) * _ExtentionSize);

	if(_Buffer->m_Ptr == NULL)
		return -2;

	_Buffer->m_Dynamic = _IsDynamic;
	_Buffer->m_ReadPtr = _Buffer->m_Ptr;
	_Buffer->m_WritePtr = _Buffer->m_Ptr;
	_Buffer->m_ExtentionSize = _ExtentionSize;
	_Buffer->m_Size = _ExtentionSize;

	_Buffer->m_BytesLeft = 0;
	
	return 0;
}

int Buffer_Extend(Buffer* _Buffer)
{
	if(_Buffer->m_Dynamic == False)
		return -2;

	int size = _Buffer->m_Size + _Buffer->m_ExtentionSize;
	unsigned char* newPtr = (unsigned char*)Allocator_Malloc(size);

	if(newPtr == NULL)
		return -1;

	if(Memory_Copy(newPtr, _Buffer->m_Ptr, _Buffer->m_Size) != _Buffer->m_Size)
	{
		Allocator_Free(newPtr);
		return -3;
	}
	_Buffer->m_WritePtr = (_Buffer->m_WritePtr - _Buffer->m_Ptr) + newPtr;
	_Buffer->m_ReadPtr = (_Buffer->m_ReadPtr - _Buffer->m_Ptr) + newPtr;

	Allocator_Free(_Buffer->m_Ptr);
	_Buffer->m_Ptr = newPtr;
	_Buffer->m_Size = size;
	
	return 0;
}

void Buffer_Clear(Buffer* _Buffer)
{
	memset(_Buffer->m_Ptr, 0, _Buffer->m_Size);

	_Buffer->m_ReadPtr = _Buffer->m_Ptr;
	_Buffer->m_WritePtr = _Buffer->m_Ptr;
	_Buffer->m_BytesLeft = 0;
}

int Buffer_ReadUInt64(Buffer* _Buffer, UInt64* _Value)
{
	int n = Memory_ParseUInt64(_Buffer->m_ReadPtr, _Value);
	_Buffer->m_ReadPtr += n;
	_Buffer->m_BytesLeft -= n;
	return n;
}

int Buffer_ReadUInt32(Buffer* _Buffer, UInt32* _Value)
{
	int n = Memory_ParseUInt32(_Buffer->m_ReadPtr, _Value);
	_Buffer->m_ReadPtr += n;
	return n;
}

int Buffer_ReadUInt16(Buffer* _Buffer, UInt16* _Value)
{
	int n = Memory_ParseUInt16(_Buffer->m_ReadPtr, _Value);
	_Buffer->m_ReadPtr += n;
	_Buffer->m_BytesLeft -= n;
	return n;
}

int Buffer_ReadUInt8(Buffer* _Buffer, UInt8* _Value)
{
	int n = Memory_ParseUInt8(_Buffer->m_ReadPtr, _Value);
	_Buffer->m_ReadPtr += n;
	_Buffer->m_BytesLeft -= n;
	return n;
}

int Buffer_ReadBuffer(Buffer* _Buffer, UInt8* _Ptr, int _Size)
{
	int readBytes = 0;

	for (int i = 0; i < _Size; i++)
		readBytes += Buffer_ReadUInt8(_Buffer, &_Ptr[i]);

	return readBytes;
}
int Buffer_WriteUInt64(Buffer* _Buffer, UInt64 _Value)
{
	int n = Memory_UInt64ToBuffer(&_Value, _Buffer->m_WritePtr);
	_Buffer->m_WritePtr += n;
	_Buffer->m_BytesLeft += n;
	return n;
}

int Buffer_WriteUInt32(Buffer* _Buffer, UInt32 _Value)
{
	int n = Memory_UInt32ToBuffer(&_Value, _Buffer->m_WritePtr);
	_Buffer->m_WritePtr += n;
	_Buffer->m_BytesLeft += n;
	return n;
}

int Buffer_WriteUInt16(Buffer* _Buffer, UInt16 _Value)
{
	int n = Memory_UInt16ToBuffer(&_Value, _Buffer->m_WritePtr);
	_Buffer->m_WritePtr += n;
	_Buffer->m_BytesLeft += n;
	return n;
}

int Buffer_WriteUInt8(Buffer* _Buffer, UInt8 _Value)
{
	int n = Memory_UInt8ToBuffer(&_Value, _Buffer->m_WritePtr);
	_Buffer->m_WritePtr += n;
	_Buffer->m_BytesLeft += n;
	return n;
}

int Buffer_WriteBuffer(Buffer* _Buffer, UInt8* _Ptr, int _Size)
{
	if(_Buffer->m_Size < _Buffer->m_WritePtr - _Buffer->m_Ptr + _Size)
	{
		if(Buffer_Extend(_Buffer) != 0)
			return -1;
	}

	int readBytes = 0;

	for (int i = 0; i < _Size; i++)
		readBytes += Buffer_WriteUInt8(_Buffer, _Ptr[i]);

	return readBytes;
}

int Buffer_ReadFromFile(Buffer* _Buffer, FILE* _File)
{
	int size = File_GetSize(_File);
	if(_Buffer->m_Size - _Buffer->m_BytesLeft < size)
		return -1;

	int readBytes = File_ReadAll(_File, _Buffer->m_WritePtr, size);
	_Buffer->m_WritePtr += readBytes;
	_Buffer->m_BytesLeft += readBytes;

	return readBytes;
}

int Buffer_Copy(Buffer* _Des, Buffer* _Src, int _Size)
{

	if(_Size == 0)
		_Size = _Src->m_BytesLeft;
		
	if(_Des->m_Size < abs(_Des->m_Ptr - _Des->m_WritePtr) + _Size)
	{
		if(Buffer_Extend(_Des) != 0)
			return -1;
	}

	Buffer_Clear(_Des);

	return Buffer_WriteBuffer(_Des, _Src->m_ReadPtr, _Size);
}


void Buffer_Dispose(Buffer* _Buffer)
{
	if(_Buffer->m_Ptr != NULL)
	{
		Allocator_Free(_Buffer->m_Ptr);
		_Buffer->m_Ptr = NULL;
	}

	if(_Buffer->m_Allocated == True)
		Allocator_Free(_Buffer);
	else
		memset(_Buffer, 0, sizeof(Buffer));

}
