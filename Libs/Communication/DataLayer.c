#include "DataLayer.h"

int DataLayer_ReceiveMessage(DataLayer* _DataLayer);
int DataLayer_SendMessage(DataLayer* _DataLayer, Payload* _Payload);

int DataLayer_CalculateSize(DataLayer* _DataLayer);

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
		if(success < 0)
		{
			printf("DataLayer_ReceiveMessage failed\n\r");
			printf("Error code: %i\n\r", success);
		}
		
		Payload* message = NULL;
		if(_DataLayer->m_FuncOut.m_Send(_DataLayer->m_FuncOut.m_Context, &message) == 1) //Whant to send message
		{
			//send message/payload
			if(DataLayer_SendMessage(_DataLayer, message) != 0)
				message->m_State = Payload_State_Failed;
			
			else
				message->m_State = Payload_State_Sented;

		}
	}
}

int DataLayer_SendMessage(DataLayer* _DataLayer, Payload* _Payload)
{
	UInt8 CRC = 0;
	DataLayer_GetCRC(_Payload->m_Data.m_Ptr, _Payload->m_Data.m_BytesLeft, &CRC);
	
	Buffer_WriteUInt8(&_Payload->m_Data, CRC);
	
	Payload_Print(_Payload, "Datalayer", True);

	int success = _DataLayer->m_OnWrite(_DataLayer->m_DataContext, &_Payload->m_Data, _Payload->m_Data.m_BytesLeft);
	if(success < 0)
	{
		printf("DataLayer_SendMessage: OnWrite Error\n\r");
		printf("Error code: %i\n\r", success);
		return -2;
	}

	return 0;
}

int DataLayer_ReceiveMessage(DataLayer* _DataLayer)
{
	int readed = 0;
	if(_DataLayer->m_DataBuffer.m_BytesLeft != 0)
	{
		printf("Error!\r\n");
		printf("Have not read all in buffer(DataLayer)\r\n");
		readed = _DataLayer->m_DataBuffer.m_BytesLeft;
	}
	else
	{
		Buffer_Clear(&_DataLayer->m_DataBuffer);
		readed = _DataLayer->m_OnRead(_DataLayer->m_DataContext, &_DataLayer->m_DataBuffer, Payload_BufferSize);
	}
	
	if(readed > 0)
	{
		int size = DataLayer_CalculateSize(_DataLayer);

		UInt8 CRC = 0;
		UInt8 ownCRC = 0;
		Memory_ParseUInt8(&_DataLayer->m_DataBuffer.m_ReadPtr[size - 1], &CRC);
		
		DataLayer_GetCRC(_DataLayer->m_DataBuffer.m_ReadPtr, size - 1, &ownCRC);

		printf("Data(R): %i of %i\r\n", size, readed);
		for (int i = 0; i < size; i++)
			printf("%x ", _DataLayer->m_DataBuffer.m_ReadPtr[i]);
		printf("\n\r");

		if(ownCRC != CRC)
		{
			printf("CRC check Failed!\n\r");
			printf("Own CRC: %u\n\rPayloads CRC: %u\n\r", ownCRC, CRC);
			char str[UUID_STRING_SIZE];
			uuid_ToString(&_DataLayer->m_DataBuffer.m_ReadPtr[1], str);
			printf("Discarding message: %s\n\r", str);
			_DataLayer->m_DataBuffer.m_ReadPtr += size;
			_DataLayer->m_DataBuffer.m_BytesLeft -= size;
			return -1;
		}

		if(_DataLayer->m_FuncOut.m_Receive != NULL)
		{
			Payload packet;
			Payload_Initialize(&packet, NULL);

			if(Buffer_Copy(&packet.m_Data, &_DataLayer->m_DataBuffer, size - 1) < 0)
			{
				printf("Buffer copy error\n\r");
				return -2;
			}

			Buffer_ReadUInt8(&_DataLayer->m_DataBuffer, &CRC);

			Payload replay;
			Payload_Initialize(&replay, packet.m_UUID);
			if(_DataLayer->m_FuncOut.m_Receive(_DataLayer->m_FuncOut.m_Context, &packet, &replay) == 1)//Whants to send replay
			{
				//send message/payload
				int success = DataLayer_SendMessage(_DataLayer, &replay);
				if(success != 0)
				{
					printf("Replay failed\n\r");
					printf("Error code: %i\n\r", success);

					Payload_Dispose(&replay);
					Payload_Dispose(&packet);
					return -1;
				}
			}

			Payload_Dispose(&replay);
			Payload_Dispose(&packet);

		}
		
		return readed;
	}

	return 0;
}

int DataLayer_CalculateSize(DataLayer* _DataLayer)
{
	unsigned char* ptr = _DataLayer->m_DataBuffer.m_ReadPtr;
	Byte flags = 0;
	UInt16 size = 0;

	ptr += Memory_ParseUInt8(ptr, (UInt8*)&flags);
	ptr += UUID_DATA_SIZE + 1 + 8;// UUID, type and time

	ptr += (1 + 6)*2; //Src & Des
	
	if(BitHelper_GetBit(&flags, 2) == True)
	{
		ptr++;
		ptr += Memory_ParseUInt16(ptr, &size);
		ptr += size;
	}
	
	ptr += Memory_ParseUInt16(ptr, &size);
	ptr += size;

	return ptr - _DataLayer->m_DataBuffer.m_ReadPtr + 1;
}

void DataLayer_Dispose(DataLayer* _DataLayer)
{
	Buffer_Dispose(&_DataLayer->m_DataBuffer);

	if(_DataLayer->m_Allocated == True)
		Allocator_Free(_DataLayer);
	else
		memset(_DataLayer, 0, sizeof(DataLayer));

}