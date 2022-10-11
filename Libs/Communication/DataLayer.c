#include "DataLayer.h"

int DataLayer_InitializePtr(int (*_OnConnect)(void* _Context), int (*_OnRead)(void* _Context, Buffer* _Buffer, int _Size), int (*_OnWrite)(void* _Context, Buffer* _Buffer, int _Size), int (*_OnDisconnect)(void* _Context), void* _DataContext, UInt64 _Timeout, DataLayer** _DataLayerPtr)
{
	DataLayer* _DataLayer = (DataLayer*)Allocator_Malloc(sizeof(DataLayer));
	if(_DataLayer == NULL)
		return -1;
	
	int success = DataLayer_Initialize(_DataLayer, _OnConnect, _OnRead, _OnWrite, _OnDisconnect, _DataContext, _Timeout);
	if(success != 0)
	{
		Allocator_Free(_DataLayer);
		return success;
	}
	
	_DataLayer->m_Allocated = True;
	
	*(_DataLayerPtr) = _DataLayer;
	return 0;
}

int DataLayer_Initialize(DataLayer* _DataLayer, int (*_OnConnect)(void* _Context), int (*_OnRead)(void* _Context, Buffer* _Buffer, int _Size), int (*_OnWrite)(void* _Context, Buffer* _Buffer, int _Size), int (*_OnDisconnect)(void* _Context), void* _DataContext, UInt64 _Timeout)
{
	_DataLayer->m_Allocated = False;
	_DataLayer->m_DataContext = _DataContext;
	_DataLayer->m_OnConnect = _OnConnect;
	_DataLayer->m_OnRead = _OnRead;
	_DataLayer->m_OnWrite = _OnWrite;
	_DataLayer->m_OnDisconnect = _OnDisconnect;

	_DataLayer->m_Timeout = _Timeout;
	// _DataLayer->m_NextHeartbeat = 0;
	_DataLayer->m_NextTimeout = 0;

	Payload_FuncOut_Clear(&_DataLayer->m_FuncOut);

	int success = Buffer_Initialize(&_DataLayer->m_DataBuffer, Payload_BufferSize);
	if(success != 0)
	{
		printf("Failed to initialize the DataBuffer!\n\r");
		printf("Error code: %i\n\r", success);
		return -2;
	}
	
	return 0;
}

void DataLayer_Work(UInt64 _MSTime, DataLayer* _DataLayer)
{
	if(_DataLayer->m_NextTimeout < _MSTime)
	{
		_DataLayer->m_NextTimeout = _MSTime + _DataLayer->m_Timeout;
		Buffer_Clear(&_DataLayer->m_DataBuffer);
		if(_DataLayer->m_OnRead(_DataLayer->m_DataContext, &_DataLayer->m_DataBuffer, Payload_BufferSize) > 0)
		{
			Payload Payload;

			Buffer_ReadUInt16(&_DataLayer->m_DataBuffer, &Payload.m_Size);
			Buffer_ReadBuffer(&_DataLayer->m_DataBuffer, (UInt8*)Payload.m_Data.BUFFER, Payload.m_Size);

			UInt8 CRC = 0;
			UInt8 ownCRC = 0;
			Buffer_ReadUInt8(&_DataLayer->m_DataBuffer, &CRC);
			
			DataLayer_GetCRC((unsigned char*)&_DataLayer->m_DataBuffer.m_Ptr, Payload.m_Size + 2, &ownCRC);

			if(ownCRC != CRC)
			{
				printf("CRC check Failed!\n\r");
				printf("Own CRC: %u\n\rPayloads CRC: %u\n\r", ownCRC, CRC);
				return;
			}
			else
			{
				printf("CRC: %u\n\r", CRC);
			}

			if(_DataLayer->m_FuncOut.m_Receive != NULL)
				_DataLayer->m_FuncOut.m_Receive(_DataLayer->m_FuncOut.m_Context, &Payload);
		}
	}
}


void DataLayer_Dispose(DataLayer* _DataLayer)
{
	Buffer_Dispose(&_DataLayer->m_DataBuffer);

	if(_DataLayer->m_Allocated == True)
		Allocator_Free(_DataLayer);
	else
		memset(_DataLayer, 0, sizeof(DataLayer));

}