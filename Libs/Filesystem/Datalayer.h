#ifndef Datalayer_h__
#define Datalayer_h__

struct T_Datalayer;
typedef struct T_Datalayer Datalayer;

#include "../BitHelper.h"

#ifndef Datalayer_CRC
	#define Datalayer_CRC (0b11011)
#endif

#ifndef Datalayer_BufferSize
	#define Datalayer_BufferSize 256
#endif

typedef struct
{
	UInt8 size;
	union 
	{
		UInt8 UI8;
		UInt16 UI16;
		UInt32 UI32;
		UInt64 UI64;
		unsigned char BUFFER[Datalayer_BufferSize];

		//TODO: Add UUID
		// UInt8 UUID[UUID_DATA_SIZE];
		UInt8 IP[4];
		UInt8 MAC[6];
	} u;
	
} Datalayer_Paylode;

struct T_Datalayer
{
	Bool m_Allocated;
};



static inline void Datalayer_GetCRC(unsigned char* _Data, UInt8* _Result)
{
	unsigned char* ptr = _Data;
	UInt8 CRC = Datalayer_CRC;
	BitHelper_SetBit(&CRC, 4, False);
	UInt16 result = 0;
	ptr += Memory_ParseUInt8(ptr, (UInt8*)&result);
	result = result << 4;


	for (int i = 11; i > 4; i--)
	{
		Bool bit = BitHelper_Get16Bit(&result, i);
		BitHelper_Set16Bit(&result, i, False);
		if(bit == True)
		{
			result = result ^ (CRC << (i - 4));
		}
	}

	if(_Result != NULL)
		*(_Result) = (UInt8)(result & 0b1111);
}
int Datalayer_InitializePtr(Datalayer** _DatalayerPtr);
int Datalayer_Initialize(Datalayer* _Datalayer);

void Datalayer_Dispose(Datalayer* _Datalayer);
#endif // Datalayer_h__