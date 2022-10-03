#include "Memory.h"

int Memory_UInt32ToBuffer(u_int32_t* _Src, unsigned char* _Pointer)
{
	_Pointer[0] = (unsigned char)((*_Src >> 24) & 0xFF);
	_Pointer[1] = (unsigned char)((*_Src >> 16) & 0xFF);
	_Pointer[2] = (unsigned char)((*_Src >> 8) & 0xFF);
	_Pointer[3] = (unsigned char)((*_Src & 0xFF));
	return 4;
}

int Memory_UInt16ToBuffer(u_int16_t* _Src, unsigned char* _Pointer)
{
	_Pointer[0] = (unsigned char)((*_Src >> 8) & 0xFF);
	_Pointer[1] = (unsigned char)((*_Src & 0xFF));
	return 4;
}

int Memory_UInt8ToBuffer(u_int8_t* _Src, unsigned char* _Pointer)
{
	_Pointer[0] = (unsigned char)((*_Src & 0xFF));
	return 4;
}

int Memory_ParseUInt32(const void* _Pointer, u_int32_t* _Dest)
{
	*_Dest = 0;
	*_Dest = (u_int32_t)((unsigned char*)_Pointer)[0]; *_Dest <<= 8;
	*_Dest |= (u_int32_t)((unsigned char*)_Pointer)[1]; *_Dest <<= 8;
	*_Dest |= (u_int32_t)((unsigned char*)_Pointer)[2]; *_Dest <<= 8;
	*_Dest |= (u_int32_t)((unsigned char*)_Pointer)[3];
	return 4;
}

int Memory_ParseUInt16(const void* _Pointer, u_int16_t* _Dest)
{
	*_Dest = 0;
	*_Dest |= (u_int16_t)((unsigned char*)_Pointer)[0]; *_Dest <<= 8;
	*_Dest |= (u_int16_t)((unsigned char*)_Pointer)[1];
	return 4;
}

int Memory_ParseUInt8(const void* _Pointer, u_int8_t* _Dest)
{
	*_Dest = 0;
	*_Dest |= (u_int8_t)((unsigned char*)_Pointer)[0];
	return 4;
}
