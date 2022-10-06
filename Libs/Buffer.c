#include "Buffer.h"

int Buffer_InitializePtr(int _Size, Buffer** _BufferPtr)
{
	Buffer* _Buffer = (Buffer*)Allocator_Malloc(sizeof(Buffer));
	if(_Buffer == NULL)
		return -1;
	
	int success = Buffer_Initialize(_Buffer, _Size);
	if(success != 0)
	{
		Allocator_Free(_Buffer);
		return success;
	}
	
	_Buffer->m_Allocated = True;
	
	*(_BufferPtr) = _Buffer;
	return 0;
}

int Buffer_Initialize(Buffer* _Buffer, int _Size)
{
	_Buffer->m_Allocated = False;

	_Buffer->m_Ptr = (unsigned char*)Allocator_Malloc(sizeof(unsigned char) * _Size);

	if(_Buffer->m_Ptr == NULL)
		return -2;

	_Buffer->m_ReadPtr = _Buffer->m_Ptr;
	_Buffer->m_WritePtr = _Buffer->m_Ptr;
	_Buffer->m_Size = _Size;
	
	return 0;
}

int Buffer_ReadUInt64(Buffer* _Buffer, UInt64* _Value)
{
	int n = Memory_ParseUInt64(_Buffer->m_ReadPtr, _Value);
	_Buffer->m_ReadPtr += n;
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
	return n;
}

int Buffer_ReadUInt8(Buffer* _Buffer, UInt8* _Value)
{
	int n = Memory_ParseUInt8(_Buffer->m_ReadPtr, _Value);
	_Buffer->m_ReadPtr += n;
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
	return n;
}

int Buffer_WriteUInt32(Buffer* _Buffer, UInt32 _Value)
{
	int n = Memory_UInt32ToBuffer(&_Value, _Buffer->m_WritePtr);
	_Buffer->m_WritePtr += n;
	return n;
}

int Buffer_WriteUInt16(Buffer* _Buffer, UInt16 _Value)
{
	int n = Memory_UInt16ToBuffer(&_Value, _Buffer->m_WritePtr);
	_Buffer->m_WritePtr += n;
	return n;
}

int Buffer_WriteUInt8(Buffer* _Buffer, UInt8 _Value)
{
	int n = Memory_UInt8ToBuffer(&_Value, _Buffer->m_WritePtr);
	_Buffer->m_WritePtr += n;
	return n;
}

int Buffer_WriteBuffer(Buffer* _Buffer, UInt8* _Ptr, int _Size)
{
	int readBytes = 0;

	for (int i = 0; i < _Size; i++)
		readBytes += Buffer_WriteUInt8(_Buffer, _Ptr[i]);

	return readBytes;
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