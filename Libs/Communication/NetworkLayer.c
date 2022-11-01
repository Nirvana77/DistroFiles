#include "NetworkLayer.h"

int NetworkLayer_PayloadLinker(NetworkLayer* _NetworLayer, Payload* _Dst, Payload* _Src);
int NetworkLayer_PayloadBuilder(NetworkLayer* _NetworLayer, Payload* _Payload);

int NetworkLayer_InitializePtr(NetworkLayer** _NetworkLayerPtr)
{
	NetworkLayer* _NetworkLayer = (NetworkLayer*)Allocator_Malloc(sizeof(NetworkLayer));
	if(_NetworkLayer == NULL)
		return -1;
	
	int success = NetworkLayer_Initialize(_NetworkLayer);
	if(success != 0)
	{
		Allocator_Free(_NetworkLayer);
		return success;
	}
	
	_NetworkLayer->m_Allocated = True;
	
	*(_NetworkLayerPtr) = _NetworkLayer;
	return 0;
}

int NetworkLayer_Initialize(NetworkLayer* _NetworkLayer)
{
	_NetworkLayer->m_Allocated = False;
	
	return 0;
}

int NetworkLayer_SendPayload(void* _Context, Payload** _PaylodePtr)
{
	NetworkLayer* _NetworkLayer = (NetworkLayer*) _Context;
	if(_NetworkLayer->m_FuncOut.m_Send == NULL)
		return 0;


	Payload* message = NULL;
	if(_NetworkLayer->m_FuncOut.m_Send(_NetworkLayer->m_FuncOut.m_Context, &message) == 1) //Whant to send meesage
	{
		message->m_Src.m_Type = Payload_Address_Type_MAC;
		GetMAC(message->m_Src.m_Address.MAC);

		printf("NetworkLayer_SendPayload\n\r");
		int success = NetworkLayer_PayloadBuilder(_NetworkLayer, message);
		if(success == 0)
		{
			*(_PaylodePtr) = message;
			return 1;
		}

	}

	return 0;
}

int NetworkLayer_ReveicePayload(void* _Context, Payload* _Message, Payload* _Replay)
{
	NetworkLayer* _NetworkLayer = (NetworkLayer*) _Context;

	printf("NetworkLayer_ReveicePayload\n\r");
	
	Buffer_ReadUInt8(&_Message->m_Data, (UInt8*)&_Message->m_Type);

	Buffer_ReadUInt64(&_Message->m_Data, &_Message->m_Time);

	UInt8 type = 0;
	Buffer_ReadUInt8(&_Message->m_Data, (UInt8*)&type);
	_Message->m_Src.m_Type = (Payload_Address_Type) type;
	Payload_ReadCommunicator(&_Message->m_Src, &_Message->m_Data);

	Buffer_ReadUInt8(&_Message->m_Data, (UInt8*)&type);
	_Message->m_Des.m_Type = (Payload_Address_Type) type;
	Payload_ReadCommunicator(&_Message->m_Des, &_Message->m_Data);

	if(_Message->m_Des.m_Type == Payload_Address_Type_IP)
	{
		UInt8 addrass[4];
		GetIP(addrass);
		if(CommperIP(addrass, _Message->m_Des.m_Address.IP) == False)
			return 0;
		
	}
	else if(_Message->m_Des.m_Type == Payload_Address_Type_MAC)
	{
		UInt8 mac[6];
		GetMAC(mac);
		if(CommperMAC(mac, _Message->m_Des.m_Address.MAC) == False)
			return 0;
	}

	Payload_ReadMessage(&_Message->m_Message, &_Message->m_Data);

	Buffer_ReadUInt16(&_Message->m_Data, &_Message->m_Size);

	if(_NetworkLayer->m_FuncOut.m_Receive != NULL)
	{
		Payload replay;
		Payload_Initialize(&replay);
		
		replay.m_Src.m_Type = Payload_Address_Type_IP;
		GetIP(replay.m_Src.m_Address.IP);
		
		if(_NetworkLayer->m_FuncOut.m_Receive(_NetworkLayer->m_FuncOut.m_Context, _Message, &replay) == 1)
		{//Whant to send replay
			printf("NetworkLayer_ReveicePayload_Replay\n\r");

			Payload_FilCommunicator(&replay.m_Des, &_Message->m_Src);

			int success = NetworkLayer_PayloadLinker(_NetworkLayer, _Replay, &replay);
			if(success != 0)
			{
				Payload_Dispose(&replay);
				return -1;

			}
			Payload_Copy(_Replay, &replay);
			Payload_Dispose(&replay);
			return 1;
		}
		Payload_Dispose(&replay);
	}

	return 0;
}

int NetworkLayer_PayloadBuilder(NetworkLayer* _NetworLayer, Payload* _Payload)
{
	Buffer temp;
	Buffer_Initialize(&temp, False, _Payload->m_Size);
	Buffer_Copy(&temp, &_Payload->m_Data, _Payload->m_Size);

	Buffer_Clear(&_Payload->m_Data);

	Byte flags = 0;
	BitHelper_SetBit(&flags, 0, _Payload->m_Src.m_Type == Payload_Address_Type_NONE ? False : True);
	BitHelper_SetBit(&flags, 1, _Payload->m_Des.m_Type == Payload_Address_Type_NONE ? False : True);
	BitHelper_SetBit(&flags, 2, _Payload->m_Message.m_Type == Payload_Message_Type_None ? False : True);

	Buffer_WriteUInt8(&_Payload->m_Data, flags);

	int success = Buffer_WriteUInt8(&_Payload->m_Data, _Payload->m_Type);
	if(success < 0)
		return -1;

	success = Buffer_WriteUInt64(&_Payload->m_Data, _Payload->m_Time);
	if(success < 0)
		return -2;

	if(_Payload->m_Src.m_Type == Payload_Address_Type_NONE)
		_Payload->m_Src.m_Type = Payload_Address_Type_MAC;
	
	UInt8 address[6];
	if(_Payload->m_Src.m_Type == Payload_Address_Type_IP)
		GetIP(&_Payload->m_Src.m_Address);
	
	else if(_Payload->m_Src.m_Type == Payload_Address_Type_MAC)
		GetMAC(&_Payload->m_Src.m_Address);
	
	success = Payload_WriteCommunicator(&_Payload->m_Src, &_Payload->m_Data);
	if(success < 0)
		return -4;
	
	success = Payload_WriteCommunicator(&_Payload->m_Des, &_Payload->m_Data);
	if(success < 0)
		return -6;

	success = Payload_WriteMessage(&_Payload->m_Message, &_Payload->m_Data);
	if(success < 0)
		return -7;
	
	success = Buffer_WriteUInt16(&_Payload->m_Data, _Payload->m_Size);
	if(success < 0)
		return -8;

	success = Buffer_WriteBuffer(&_Payload->m_Data, temp.m_ReadPtr, temp.m_BytesLeft);
	if(success < 0)
		return -9;

	Buffer_Dispose(&temp);
	return 0;
}

int NetworkLayer_PayloadLinker(NetworkLayer* _NetworLayer, Payload* _Dst, Payload* _Src)
{

	int success = Buffer_WriteUInt8(&_Dst->m_Data, _Src->m_Type);
	if(success < 0)
		return -1;

	success = Buffer_WriteUInt64(&_Dst->m_Data, _Src->m_Time);
	if(success < 0)
		return -2;

	if(_Src->m_Src.m_Type == Payload_Address_Type_NONE)
		_Src->m_Src.m_Type = Payload_Address_Type_MAC;

	success = Buffer_WriteUInt8(&_Dst->m_Data, _Src->m_Src.m_Type);
	if(success < 0)
		return -3;
	
	if(_Src->m_Src.m_Type == Payload_Address_Type_IP)
	{
		UInt8 address[4];
		GetIP(address);
		Buffer_WriteBuffer(&_Dst->m_Data, address, 4);
	}
	else if(_Src->m_Src.m_Type == Payload_Address_Type_MAC)
	{
		UInt8 mac[6];
		GetMAC(mac);
		Buffer_WriteBuffer(&_Dst->m_Data, mac, 6);
	}
	/*
	success = Payload_WriteCommunicator(&_Src->m_Src, &_Dst->m_Data);
	if(success < 0)
		return -4;
	*/
	success = Payload_WriteCommunicator(&_Src->m_Des, &_Dst->m_Data);
	if(success < 0)
		return -6;

	success = Payload_WriteMessage(&_Src->m_Message, &_Dst->m_Data);
	if(success < 0)
		return -7;
	
	success = Buffer_WriteUInt16(&_Dst->m_Data, _Src->m_Size);
	if(success < 0)
		return -8;
	
	success = Buffer_WriteBuffer(&_Dst->m_Data, _Src->m_Data.m_ReadPtr, _Src->m_Data.m_BytesLeft);
	if(success < 0)
		return -8;
 
	return 0;
}


void NetworkLayer_Dispose(NetworkLayer* _NetworkLayer)
{
	

	if(_NetworkLayer->m_Allocated == True)
		Allocator_Free(_NetworkLayer);
	else
		memset(_NetworkLayer, 0, sizeof(NetworkLayer));

}