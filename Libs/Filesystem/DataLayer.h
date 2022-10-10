#ifndef DataLayer_h__
#define DataLayer_h__

struct T_DataLayer;
typedef struct T_DataLayer DataLayer;

#include "../BitHelper.h"

#ifndef DataLayer_CRC
	#define DataLayer_CRC (0b11011)
#endif

#ifndef DataLayer_BufferSize
	#define DataLayer_BufferSize 256
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
		unsigned char BUFFER[DataLayer_BufferSize];

		//TODO: Add UUID
		// UInt8 UUID[UUID_DATA_SIZE];
		UInt8 IP[4];
		UInt8 MAC[6];
	} u;
	
} DataLayer_Paylode;

struct T_DataLayer
{
	Bool m_Allocated;

	void* m_DataContext;
	int (*m_OnRead)(void* _Context, Buffer* _Buffer, int _Size);
	int (*m_OnWrite)(void* _Context, Buffer* _Buffer, int _Size);
	int (*m_OnConnect)(void* _Context);
	int (*m_OnDisconnect)(void* _Context);

	UInt64 m_Timeout;
	UInt64 m_NextHeartbeat;
	UInt64 m_LastTimeout;

};



static inline void DataLayer_GetCRC(unsigned char* _Data, int _Size, UInt8* _Result)
{
	unsigned char* ptr = _Data;
	UInt8 CRC = DataLayer_CRC;
	BitHelper_SetBit(&CRC, 4, False);
	UInt16 result = 0;
	
	for (int i = 0; i < _Size; i++)
	{
		UInt8 data;
		ptr += Memory_ParseUInt8(ptr, &data);
		result = result + data << 4;

		for (int j = 11; j > 4; j--)
		{
			Bool bit = BitHelper_Get16Bit(&result, j);
			BitHelper_Set16Bit(&result, j, False);
			if(bit == True)
			{
				result = result ^ (CRC << (j - 4));
			}
		}
	}
	

	if(_Result != NULL)
		*(_Result) = (UInt8)(result);
}

int DataLayer_InitializePtr(int (*_OnConnect)(void* _Context), int (*_OnRead)(void* _Context, Buffer* _Buffer, int _Size), int (*_OnWrite)(void* _Context, Buffer* _Buffer, int _Size), int (*_OnDisconnect)(void* _Context), void* _DataContext, UInt64 _Timeout, DataLayer** _DataLayerPtr);
int DataLayer_Initialize(DataLayer* _DataLayer, int (*_OnConnect)(void* _Context), int (*_OnRead)(void* _Context, Buffer* _Buffer, int _Size), int (*_OnWrite)(void* _Context, Buffer* _Buffer, int _Size), int (*_OnDisconnect)(void* _Context), void* _DataContext, UInt64 _Timeout);

void DataLayer_Work(UInt64 _MSTime, DataLayer* _DataLayer);

void DataLayer_Dispose(DataLayer* _DataLayer);
#endif // DataLayer_h__