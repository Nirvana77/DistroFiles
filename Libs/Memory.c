#include "Memory.h"

int Memory_UInt64ToBuffer(UInt64* _Src, unsigned char* _Pointer)
{
	_Pointer[0] = (unsigned char)((*_Src >> 56) & 0xFF);
	_Pointer[1] = (unsigned char)((*_Src >> 48) & 0xFF);
	_Pointer[2] = (unsigned char)((*_Src >> 40) & 0xFF);
	_Pointer[3] = (unsigned char)((*_Src >> 32) & 0xFF);
	_Pointer[4] = (unsigned char)((*_Src >> 24) & 0xFF);
	_Pointer[5] = (unsigned char)((*_Src >> 16) & 0xFF);
	_Pointer[6] = (unsigned char)((*_Src >> 8) & 0xFF);
	_Pointer[7] = (unsigned char)((*_Src & 0xFF));
	return 8;
}

int Memory_UInt32ToBuffer(UInt32* _Src, unsigned char* _Pointer)
{
	_Pointer[0] = (unsigned char)((*_Src >> 24) & 0xFF);
	_Pointer[1] = (unsigned char)((*_Src >> 16) & 0xFF);
	_Pointer[2] = (unsigned char)((*_Src >> 8) & 0xFF);
	_Pointer[3] = (unsigned char)((*_Src & 0xFF));
	return 4;
}

int Memory_UInt16ToBuffer(UInt16* _Src, unsigned char* _Pointer)
{
	_Pointer[0] = (unsigned char)((*_Src >> 8) & 0xFF);
	_Pointer[1] = (unsigned char)((*_Src & 0xFF));
	return 2;
}

int Memory_UInt8ToBuffer(UInt8* _Src, unsigned char* _Pointer)
{
	_Pointer[0] = (unsigned char)((*_Src & 0xFF));
	return 1;
}

int Memory_ParseUInt64(const void* _Pointer, UInt64* _Dest)
{
	*_Dest = 0;
	*_Dest = (UInt64)((unsigned char*)_Pointer)[0]; *_Dest <<= 8;
	*_Dest |= (UInt64)((unsigned char*)_Pointer)[1]; *_Dest <<= 8;
	*_Dest |= (UInt64)((unsigned char*)_Pointer)[2]; *_Dest <<= 8;
	*_Dest |= (UInt64)((unsigned char*)_Pointer)[3]; *_Dest <<= 8;
	*_Dest |= (UInt64)((unsigned char*)_Pointer)[4]; *_Dest <<= 8;
	*_Dest |= (UInt64)((unsigned char*)_Pointer)[5]; *_Dest <<= 8;
	*_Dest |= (UInt64)((unsigned char*)_Pointer)[6]; *_Dest <<= 8;
	*_Dest |= (UInt64)((unsigned char*)_Pointer)[7];
	return 8;
}

int Memory_ParseUInt32(const void* _Pointer, UInt32* _Dest)
{
	*_Dest = 0;
	*_Dest = (UInt32)((unsigned char*)_Pointer)[0]; *_Dest <<= 8;
	*_Dest |= (UInt32)((unsigned char*)_Pointer)[1]; *_Dest <<= 8;
	*_Dest |= (UInt32)((unsigned char*)_Pointer)[2]; *_Dest <<= 8;
	*_Dest |= (UInt32)((unsigned char*)_Pointer)[3];
	return 4;
}

int Memory_ParseUInt16(const void* _Pointer, UInt16* _Dest)
{
	*_Dest = 0;
	*_Dest |= (UInt16)((unsigned char*)_Pointer)[0]; *_Dest <<= 8;
	*_Dest |= (UInt16)((unsigned char*)_Pointer)[1];
	return 2;
}

int Memory_ParseUInt8(const void* _Pointer, UInt8* _Dest)
{
	*_Dest = 0;
	*_Dest |= (UInt8)((unsigned char*)_Pointer)[0];
	return 1;
}

int Memory_ParseBuffer(const void* _Des, const void* _Src, int _BufferSize)
{
	int size = 0;
	int readed = 0;
	unsigned char* desPtr = (unsigned char*)_Des;
	unsigned char* srcPtr = (unsigned char*)_Src;

	for (int i = 0; i < _BufferSize; i++)
	{
		readed = Memory_ParseUInt8(srcPtr, desPtr);

		size += readed;
		desPtr += readed;
		srcPtr += readed;
	}

	return size;
}