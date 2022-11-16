#include "Buffer.h"


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


int Buffer_ExtendBy(Buffer* _Buffer, int _Size)
{
	if(_Buffer->m_Dynamic == False)
		return -2;

	if(_Size % _Buffer->m_ExtentionSize != 0)
		_Size += _Buffer->m_ExtentionSize - _Size % _Buffer->m_ExtentionSize;

	int size = _Buffer->m_Size + _Size;
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

int Buffer_ReadBuffer(Buffer* _Buffer, unsigned char* _Ptr, int _Size)
{
	int readBytes = 0;

	for (int i = 0; i < _Size; i++)
		readBytes += Buffer_ReadUInt8(_Buffer, &_Ptr[i]);

	return readBytes;
}
int Buffer_WriteUInt64(Buffer* _Buffer, UInt64 _Value)
{
	if(_Buffer->m_Size < _Buffer->m_WritePtr - _Buffer->m_Ptr + 8)
	{
		if(Buffer_Extend(_Buffer) != 0)
				return -1;
	}
	int n = Memory_UInt64ToBuffer(&_Value, _Buffer->m_WritePtr);
	_Buffer->m_WritePtr += n;
	_Buffer->m_BytesLeft += n;
	return n;
}

int Buffer_WriteUInt32(Buffer* _Buffer, UInt32 _Value)
{
	if(_Buffer->m_Size < _Buffer->m_WritePtr - _Buffer->m_Ptr + 4)
	{
		if(Buffer_Extend(_Buffer) != 0)
				return -1;
	}
	int n = Memory_UInt32ToBuffer(&_Value, _Buffer->m_WritePtr);
	_Buffer->m_WritePtr += n;
	_Buffer->m_BytesLeft += n;
	return n;
}

int Buffer_WriteUInt16(Buffer* _Buffer, UInt16 _Value)
{
	if(_Buffer->m_Size < _Buffer->m_WritePtr - _Buffer->m_Ptr + 2)
	{
		if(Buffer_Extend(_Buffer) != 0)
				return -1;
	}
	int n = Memory_UInt16ToBuffer(&_Value, _Buffer->m_WritePtr);
	_Buffer->m_WritePtr += n;
	_Buffer->m_BytesLeft += n;
	return n;
}

int Buffer_WriteUInt8(Buffer* _Buffer, UInt8 _Value)
{
	if(_Buffer->m_Size < _Buffer->m_WritePtr - _Buffer->m_Ptr + 1)
	{
		if(Buffer_Extend(_Buffer) != 0)
				return -1;
	}

	int n = Memory_UInt8ToBuffer(&_Value, _Buffer->m_WritePtr);
	_Buffer->m_WritePtr += n;
	_Buffer->m_BytesLeft += n;
	return n;
}

int Buffer_WriteBuffer(Buffer* _Buffer, unsigned char* _Ptr, int _Size)
{
	if(_Buffer->m_Size < _Buffer->m_WritePtr - _Buffer->m_Ptr + _Size)
	{
		if(Buffer_ExtendBy(_Buffer, _Size) != 0)
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
	
	if(_Buffer->m_Size < _Buffer->m_WritePtr - _Buffer->m_Ptr + size)
	{
		if(Buffer_ExtendBy(_Buffer, size) != 0)
			return -1;
	}

	int readBytes = File_ReadAll(_File, _Buffer->m_WritePtr, size);
	_Buffer->m_WritePtr += readBytes;
	_Buffer->m_BytesLeft += readBytes;

	return readBytes;
}

int Buffer_Copy(Buffer* _Des, Buffer* _Src, int _Size)
{

	if(_Size == 0)
		_Size = _Src->m_BytesLeft;
		
	if(_Des->m_Size < _Des->m_WritePtr - _Des->m_Ptr + _Size)
	{
		if(Buffer_ExtendBy(_Des, _Des->m_Size + (int)(_Des->m_WritePtr - _Des->m_Ptr + _Size)) != 0)
				return -1;
	}

	Buffer_Clear(_Des);

	int wrtten = Buffer_WriteBuffer(_Des, _Src->m_ReadPtr, _Size);

	_Src->m_ReadPtr += wrtten;
	_Src->m_BytesLeft -= wrtten;

	return 1;
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
