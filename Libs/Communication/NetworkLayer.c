#include "NetworkLayer.h"

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
	UInt8 type = 0;
	Buffer_ReadUInt8(&_Message->m_Data, &type);
	_Message->m_Type = type;
	
	type = 0;
	Buffer_ReadUInt8(&_Message->m_Data, &type);
	_Message->m_Src.m_Type = type;
	Payload_ReadAddress(&_Message->m_Src, &_Message->m_Data);
	
	type = 0;
	Buffer_ReadUInt8(&_Message->m_Data, &type);
	_Message->m_Des.m_Type = type;
	Payload_ReadAddress(&_Message->m_Des, &_Message->m_Data);

	if(_Message->m_Des.m_Type == Payload_Address_Type_IP)
	{
		UInt8 addrass[4];
		GetIP(addrass);
		if(CommperIP(addrass, _Message->m_Des.m_Address.IP) == False)
		{
			printf("Ignored IP\r\n");
			return 0;
		}
		
	}
	else if(_Message->m_Des.m_Type == Payload_Address_Type_MAC)
	{
		UInt8 mac[6];
		GetMAC(mac);
		if(CommperMAC(mac, _Message->m_Des.m_Address.MAC) == False)
		{
			printf("Ignored MAC\r\n");
			return 0;
		}
	}
	else if(_Message->m_Type != Payload_Type_Broadcast && _Message->m_Type != Payload_Type_BroadcastRespons)
	{
		printf("Ignored Not Broadcast or BroadcastRespons\r\n");
		printf("_Message Type is %i and des type is %i\r\n", _Message->m_Type, _Message->m_Des.m_Type);
		return 0;
	}

	if(_NetworkLayer->m_FuncOut.m_Receive != NULL)
	{
		Payload replay;
		Payload_Initialize(&replay, _Replay->m_UUID);
		
		replay.m_Src.m_Type = Payload_Address_Type_MAC;
		GetMAC(replay.m_Src.m_Address.MAC);
		
		if(_NetworkLayer->m_FuncOut.m_Receive(_NetworkLayer->m_FuncOut.m_Context, _Message, &replay) == 1)
		{//Whant to send replay

			Payload_FilAddress(&replay.m_Des, &_Message->m_Src);
			Payload_DeepCopy(_Replay, &replay);

			int success = NetworkLayer_PayloadBuilder(_NetworkLayer, _Replay);
			Payload_Dispose(&replay);
			if(success != 0)
				return -1;

			return 1;
		}
		Payload_Dispose(&replay);
	}

	return 0;

	/*
	Byte flags = 0;
	Buffer_ReadUInt8(&_Message->m_Data, (UInt8*)&flags);

	UInt8 UUID[UUID_DATA_SIZE];
	Buffer_ReadBuffer(&_Message->m_Data, UUID, UUID_DATA_SIZE);
	
	Buffer_ReadUInt8(&_Message->m_Data, (UInt8*)&_Message->m_Type);

	Buffer_ReadUInt64(&_Message->m_Data, &_Message->m_Time);

	UInt8 type = 0;
	Buffer_ReadUInt8(&_Message->m_Data, (UInt8*)&type);
	_Message->m_Src.m_Type = (Payload_Address_Type) type;
	Payload_ReadAddress(&_Message->m_Src, &_Message->m_Data);

	Buffer_ReadUInt8(&_Message->m_Data, (UInt8*)&type);
	_Message->m_Des.m_Type = (Payload_Address_Type) type;
	Payload_ReadAddress(&_Message->m_Des, &_Message->m_Data);

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

	Payload_Print(_Message, "ReveicePayload", True);

	if(_NetworkLayer->m_FuncOut.m_Receive != NULL)
	{
		Payload replay;
		Payload_Initialize(&replay, _Replay->m_UUID);
		
		replay.m_Src.m_Type = Payload_Address_Type_IP;
		GetIP(replay.m_Src.m_Address.IP);
		
		if(_NetworkLayer->m_FuncOut.m_Receive(_NetworkLayer->m_FuncOut.m_Context, _Message, &replay) == 1)
		{//Whant to send replay

			Payload_FilAddress(&replay.m_Des, &_Message->m_Src);
			Payload_DeepCopy(_Replay, &replay);

			int success = NetworkLayer_PayloadBuilder(_NetworkLayer, _Replay);
			Payload_Dispose(&replay);
			if(success != 0)
				return -1;

			return 1;
		}
		Payload_Dispose(&replay);
	}

	return 0;
	*/
}

int NetworkLayer_PayloadBuilder(NetworkLayer* _NetworLayer, Payload* _Payload)
{
	_Payload->m_Src.m_Type = Payload_Address_Type_MAC;
	GetMAC((UInt8*)&_Payload->m_Src.m_Address);
	/*
	Buffer temp;
	if(_Payload->m_Size != 0) 
	{
		Buffer_Initialize(&temp, _Payload->m_Size);
		Buffer_DeepCopy(&temp, &_Payload->m_Data, _Payload->m_Size);
	}

	Buffer_Clear(&_Payload->m_Data);

	Byte flags = 0;
	BitHelper_SetBit(&flags, 0, _Payload->m_Src.m_Type == Payload_Address_Type_NONE ? False : True);
	BitHelper_SetBit(&flags, 1, _Payload->m_Des.m_Type == Payload_Address_Type_NONE ? False : True);
	BitHelper_SetBit(&flags, 2, _Payload->m_Message.m_Type == Payload_Message_Type_None ? False : True);

	Buffer_WriteUInt8(&_Payload->m_Data, flags);

	Buffer_WriteBuffer(&_Payload->m_Data, _Payload->m_UUID, UUID_DATA_SIZE);

	int success = Buffer_WriteUInt8(&_Payload->m_Data, _Payload->m_Type);
	if(success < 0)
		return -1;

	success = Buffer_WriteUInt64(&_Payload->m_Data, _Payload->m_Time);
	if(success < 0)
		return -2;

	if(_Payload->m_Src.m_Type == Payload_Address_Type_NONE)
		_Payload->m_Src.m_Type = Payload_Address_Type_MAC;
	
	if(_Payload->m_Src.m_Type == Payload_Address_Type_IP)
		GetIP((UInt8*)&_Payload->m_Src.m_Address);
	
	else if(_Payload->m_Src.m_Type == Payload_Address_Type_MAC)
		GetMAC((UInt8*)&_Payload->m_Src.m_Address);
	
	success = Payload_WriteAddress(&_Payload->m_Src, &_Payload->m_Data);
	if(success < 0)
		return -4;
	
	success = Payload_WriteAddress(&_Payload->m_Des, &_Payload->m_Data);
	if(success < 0)
		return -6;

	success = Payload_WriteMessage(&_Payload->m_Message, &_Payload->m_Data);
	if(success < 0)
		return -7;
	
	success = Buffer_WriteUInt16(&_Payload->m_Data, _Payload->m_Size);
	if(success < 0)
		return -8;
		
	if(_Payload->m_Size != 0)
	{
		success = Buffer_WriteBuffer(&_Payload->m_Data, temp.m_ReadPtr, temp.m_BytesLeft);
		if(success < 0)
			return -9;
		
		Buffer_Dispose(&temp);
	}
	*/

	return 0;
}


void NetworkLayer_Dispose(NetworkLayer* _NetworkLayer)
{
	

	if(_NetworkLayer->m_Allocated == True)
		Allocator_Free(_NetworkLayer);
	else
		memset(_NetworkLayer, 0, sizeof(NetworkLayer));

}