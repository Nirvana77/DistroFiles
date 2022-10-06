#include "Memory.h"

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
	return 4;
}

int Memory_UInt8ToBuffer(UInt8* _Src, unsigned char* _Pointer)
{
	_Pointer[0] = (unsigned char)((*_Src & 0xFF));
	return 4;
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
	return 4;
}

int Memory_ParseUInt8(const void* _Pointer, UInt8* _Dest)
{
	*_Dest = 0;
	*_Dest |= (UInt8)((unsigned char*)_Pointer)[0];
	return 4;
}
