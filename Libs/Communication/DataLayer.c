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
				Payload_SetState(message, Payload_State_Failed, message);
			
			else
				Payload_SetState(message, Payload_State_Sented, message);

		}
	}
}

int DataLayer_SendMessage(DataLayer* _DataLayer, Payload* _Payload)
{
	Buffer buffer;
	Buffer_Initialize(&buffer, True, 32);

	UInt8 flags = 0;
	BitHelper_SetBit(&flags, 0, _Payload->m_Src.m_Type == Payload_Address_Type_NONE ? False : True);
	BitHelper_SetBit(&flags, 1, _Payload->m_Des.m_Type == Payload_Address_Type_NONE ? False : True);
	BitHelper_SetBit(&flags, 2, _Payload->m_Message.m_Type == Payload_Message_Type_None ? False : True);

	Buffer_WriteUInt8(&buffer, flags);
	
	Buffer_WriteUInt64(&buffer, _Payload->m_Time);

	Buffer_WriteUInt8(&buffer, _Payload->m_Type);
	
	Payload_WriteAddress(&_Payload->m_Src, &buffer);
	Payload_WriteAddress(&_Payload->m_Des, &buffer);
	
	Buffer_WriteBuffer(&buffer, _Payload->m_UUID, UUID_DATA_SIZE);

	Payload_WriteMessage(&_Payload->m_Message, &buffer);

	Buffer_WriteUInt16(&buffer, _Payload->m_Data.m_BytesLeft);
	
	Buffer_WriteBuffer(&buffer, _Payload->m_Data.m_Ptr, _Payload->m_Data.m_BytesLeft);

	UInt8 CRC = 0;
	DataLayer_GetCRC(buffer.m_Ptr, buffer.m_BytesLeft, &CRC);
	
	Buffer_WriteUInt8(&buffer, CRC);
	
	Payload_Print(_Payload, "Datalayer");

	int success = _DataLayer->m_OnWrite(_DataLayer->m_DataContext, &buffer, buffer.m_BytesLeft);
	Buffer_Dispose(&buffer);
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

		if(size > readed)
		{
			
			printf("Size check Failed!\n\r");
			printf("S/R: %i/%i\n\r", size, readed);
			printf("Discarding message: \n\r");
			for (int i = 0; i < readed; i++)
				printf("%i ", _DataLayer->m_DataBuffer.m_ReadPtr[i]);
			printf("\n\r");
			

			_DataLayer->m_DataBuffer.m_ReadPtr += readed;
			_DataLayer->m_DataBuffer.m_BytesLeft -= readed;
			return -1;
		}

		UInt8 CRC = 0;
		UInt8 ownCRC = 0;
		Memory_ParseUInt8(&_DataLayer->m_DataBuffer.m_ReadPtr[size - 1], &CRC);
		
		DataLayer_GetCRC(_DataLayer->m_DataBuffer.m_ReadPtr, size - 1, &ownCRC);

		if(ownCRC != CRC)
		{
			printf("CRC check Failed!\n\r");
			printf("Own CRC: %u\n\rPayloads CRC: %u\n\r", ownCRC, CRC);
			char str[UUID_STRING_SIZE];
			uuid_ToString(&_DataLayer->m_DataBuffer.m_ReadPtr[1], str);
			printf("Discarding message: %s\n\r", str);
			_DataLayer->m_DataBuffer.m_ReadPtr += size;
			_DataLayer->m_DataBuffer.m_BytesLeft -= size;
			return -2;
		}

		if(_DataLayer->m_FuncOut.m_Receive != NULL)
		{
			Payload frame;
			Payload_Initialize(&frame, NULL);
			Payload_SetState(&frame, Payload_State_Resived, &frame);

			if(Buffer_DeepCopy(&frame.m_Data, &_DataLayer->m_DataBuffer, size - 1) < 0)
			{
				printf("Buffer copy error\n\r");
				return -3;
			}

			Buffer_ReadUInt8(&_DataLayer->m_DataBuffer, &CRC);
			Buffer_ReadUInt8(&frame.m_Data, &CRC);

			UInt64 time = 0;
			Buffer_ReadUInt64(&frame.m_Data, &time);

			Payload replay;
			Payload_Initialize(&replay, frame.m_UUID);
			if(_DataLayer->m_FuncOut.m_Receive(_DataLayer->m_FuncOut.m_Context, &frame, &replay) == 1)//Whants to send replay
			{
				//send message/payload
				int success = DataLayer_SendMessage(_DataLayer, &replay);
				if(success != 0)
				{
					printf("Replay failed\n\r");
					printf("Error code: %i\n\r", success);

					Payload_Dispose(&replay);
					Payload_Dispose(&frame);
					return -4;
				}
			}

			Payload_Dispose(&replay);
			Payload_Dispose(&frame);

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
	ptr += Payload_MethodPosistion; //Src & Des & Time & Type & UUID
	
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