#ifndef DataLayer_h__
#define DataLayer_h__

struct T_DataLayer;
typedef struct T_DataLayer DataLayer;

#include "../BitHelper.h"
#include "../Buffer.h"
#include "Payload.h"

#ifndef DataLayer_CRC
	#define DataLayer_CRC (0b11011)
#endif

//The degree of the DataLayer_CRC polynomial is the width of the CRC, and the CRC is
//sent as one byte, so the polynomial has to fit in 0b10 to 0b111111111.
#if (DataLayer_CRC) < 0b10 || (DataLayer_CRC) > 0b111111111
	#error "DataLayer_CRC has to be a polynomial of degree 1 to 8, like 0b11011."
#endif

//The width of the CRC in bits, the degree of the DataLayer_CRC polynomial.
#define DataLayer_CRC_Width ( \
	((DataLayer_CRC) & 0b100000000) != 0 ? 8 : \
	((DataLayer_CRC) & 0b010000000) != 0 ? 7 : \
	((DataLayer_CRC) & 0b001000000) != 0 ? 6 : \
	((DataLayer_CRC) & 0b000100000) != 0 ? 5 : \
	((DataLayer_CRC) & 0b000010000) != 0 ? 4 : \
	((DataLayer_CRC) & 0b000001000) != 0 ? 3 : \
	((DataLayer_CRC) & 0b000000100) != 0 ? 2 : 1)

//The polynomial without its top bit, that bit is implicit in the division.
#define DataLayer_CRC_Divisor ((UInt8)((DataLayer_CRC) & ((0x1 << DataLayer_CRC_Width) - 1)))

#define DataLayer_HasSourceBit 0
#define DataLayer_HasDestinationBit 1
#define DataLayer_HasMethodBit 2

struct T_DataLayer
{
	Bool m_Allocated;

	void* m_DataContext;
	int (*m_OnRead)(void* _Context, Buffer* _Buffer);
	int (*m_OnWrite)(void* _Context, Buffer* _Buffer);
	int (*m_OnConnect)(void* _Context);
	int (*m_OnDisconnect)(void* _Context);

	Buffer m_DataBuffer;

	Payload_FuncOut m_FuncOut;

	UInt64 m_Timeout;
	// UInt64 m_NextHeartbeat; //? Do we need an heartbeat?
	UInt64 m_NextTimeout;

};

//Creds: https://quickbirdstudios.com/blog/validate-data-with-crc/
static inline void DataLayer_GetCRC(unsigned char* _Data, int _Size, UInt8* _Result)
{
	unsigned char* ptr = _Data;
	UInt8 CRC = DataLayer_CRC_Divisor;
	UInt16 result = 0;
	
	for (int i = 0; i < _Size; i++)
	{
		UInt8 data;
		ptr += Memory_ParseUInt8(ptr, &data);
		result = result + (data << DataLayer_CRC_Width);

		for (int j = DataLayer_CRC_Width + 7; j >= DataLayer_CRC_Width; j--)
		{
			Bool bit = BitHelper_Get16Bit(&result, j);
			BitHelper_Set16Bit(&result, j, False);
			if(bit == True)
			{
				result = result ^ (CRC << (j - DataLayer_CRC_Width));
			}
		}
	}
	

	if(_Result != NULL)
		*(_Result) = (UInt8)(result);
}

int DataLayer_InitializePtr(int (*_OnConnect)(void* _Context), int (*_OnRead)(void* _Context, Buffer* _Buffer), int (*_OnWrite)(void* _Context, Buffer* _Buffer), int (*_OnDisconnect)(void* _Context), void* _DataContext, UInt64 _Timeout, DataLayer** _DataLayerPtr);
int DataLayer_Initialize(DataLayer* _DataLayer, int (*_OnConnect)(void* _Context), int (*_OnRead)(void* _Context, Buffer* _Buffer), int (*_OnWrite)(void* _Context, Buffer* _Buffer), int (*_OnDisconnect)(void* _Context), void* _DataContext, UInt64 _Timeout);

void DataLayer_Work(UInt64 _MSTime, DataLayer* _DataLayer);

void DataLayer_Dispose(DataLayer* _DataLayer);
#endif // DataLayer_h__