#include "DataLayer.h"

int DataLayer_ReceiveMessage(DataLayer* _DataLayer);

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

	int success = Buffer_Initialize(&_DataLayer->m_DataBuffer, True, Payload_BufferSize);
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
		int success = DataLayer_ReceiveMessage(_DataLayer);
		if(success != 0)
		{
			printf("DataLayer_ReceiveMessage failed\n\r");
			printf("Error code: %i\n\r", success);
			return;
		}
	}
}

int DataLayer_SendMessage(void* _Context, Payload* _Payload)
{
	DataLayer* _DataLayer = (DataLayer*)_Context;

	Buffer_Clear(&_DataLayer->m_DataBuffer);
	if(Buffer_Copy(&_DataLayer->m_DataBuffer, &_Payload->m_Data, 0) != 0)
	{
		printf("Buffer copy error\n\r");
		return -2;
	}

	UInt8 CRC = 0;
	DataLayer_GetCRC(_DataLayer->m_DataBuffer.m_Ptr, _Payload->m_Size, &CRC);
	
	Buffer_WriteUInt8(&_DataLayer->m_DataBuffer, CRC);

	int success = _DataLayer->m_OnWrite(_DataLayer->m_DataContext, &_DataLayer->m_DataBuffer, _DataLayer->m_DataBuffer.m_BytesLeft);
	if(success < 0)
	{
		printf("DataLayer_SendMessage: OnWrite Error\n\r");
		printf("Error code: %i\n\r", success);
		return -1;
	}

	return 0;
}

int DataLayer_ReceiveMessage(DataLayer* _DataLayer)
{
	Buffer_Clear(&_DataLayer->m_DataBuffer);
	int readed = _DataLayer->m_OnRead(_DataLayer->m_DataContext, &_DataLayer->m_DataBuffer, Payload_BufferSize);
	if(readed > 0)
	{

		UInt8 CRC = 0;
		UInt8 ownCRC = 0;
		Memory_ParseUInt8(&_DataLayer->m_DataBuffer.m_Ptr[readed - 1], &CRC);
		
		DataLayer_GetCRC(_DataLayer->m_DataBuffer.m_Ptr, readed - 1, &ownCRC);

		if(ownCRC != CRC)
		{
			printf("CRC check Failed!\n\r");
			printf("Own CRC: %u\n\rPayloads CRC: %u\n\r", ownCRC, CRC);
			return -1;
		}

		if(_DataLayer->m_FuncOut.m_Receive != NULL)
		{
			Payload packet;
			Payload_Initialize(&packet);

			Buffer_Initialize(&packet.m_Data, False, readed - 1);

			if(Buffer_Copy(&packet.m_Data, &_DataLayer->m_DataBuffer, readed - 1) != 0)
			{
				printf("Buffer copy error\n\r");
				return -2;
			}

			int replayData = _DataLayer->m_FuncOut.m_Receive(_DataLayer->m_FuncOut.m_Context, &packet);

			Payload_Dispose(&packet);

		}
		
	}

	return 0;
}

void DataLayer_Dispose(DataLayer* _DataLayer)
{
	Buffer_Dispose(&_DataLayer->m_DataBuffer);

	if(_DataLayer->m_Allocated == True)
		Allocator_Free(_DataLayer);
	else
		memset(_DataLayer, 0, sizeof(DataLayer));

}