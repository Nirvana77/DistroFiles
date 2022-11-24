#ifndef BitHelper_h__
#define BitHelper_h__

#include "Types.h"

static inline void BitHelper_SetBit(Byte* _Byte, Byte _Bit, Bool _Value)
{
	if(_Value == True)
		*(_Byte) = *(_Byte) | (0x1 << _Bit);
	else
		*(_Byte) = *(_Byte) & ~(0x1 << _Bit);
}

static inline Bool BitHelper_GetBit(Byte* _Byte, Byte _Bit)
{
	return (Bool)((*(_Byte) >> _Bit) & 0x1);
}

//TODO: Make two function clear and set bit!
//* BitHelper_SetUInt16Bit
//* BitHelper_ClearUInt16Bit
static inline void BitHelper_Set16Bit(UInt16* _Byte, Byte _Bit, Bool _Value)
{
	if(_Value == True)
		*(_Byte) = *(_Byte) | (0x1 << _Bit);
	else
		*(_Byte) = *(_Byte) & ~(0x1 << _Bit);
}

static inline Bool BitHelper_Get16Bit(UInt16* _Byte, Byte _Bit)
{
	return (Bool)((*(_Byte) >> _Bit) & 0x1);
}

static inline int BitHelper_GetString(Byte _Byte, const char* _Str[11])
{
	if(_Str == NULL)
		return -1;

	char str[11];
	str[0] = '0';
	str[1] = 'x';
	str[10] = 0;

	for (int i = 0; i < 8; i++)
	{
		if(BitHelper_GetBit(&_Byte, i) == True)
			str[9 - i] = '1';
		else
			str[9 - i] = '0';
	}
	
	memcpy(_Str, str, strlen(str));
	return 0;
}

#endif // BitHelper_h__