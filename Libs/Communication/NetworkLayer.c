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
	if(type < Payload_Type_Min || type > Payload_Type_Max)
		return -1;
	
	_Message->m_Type = type;
	
	type = 0;
	Buffer_ReadUInt8(&_Message->m_Data, &type);
		
	if(type < Payload_Address_Type_Min || type > Payload_Address_Type_Max)
		return -2;
	_Message->m_Src.m_Type = type;
	Payload_ReadAddress(&_Message->m_Src, &_Message->m_Data);
	
	type = 0;
	Buffer_ReadUInt8(&_Message->m_Data, &type);
	if(type < Payload_Address_Type_Min || type > Payload_Address_Type_Max)
		return -3;
	_Message->m_Des.m_Type = type;
	Payload_ReadAddress(&_Message->m_Des, &_Message->m_Data);

	if(_Message->m_Des.m_Type == Payload_Address_Type_IP)
	{
		UInt8 addrass[4];
		GetIP(addrass);
		if(CommperIP(addrass, _Message->m_Des.m_Address.IP) == False)
		{
			return 0;
		}
		
	}
	else if(_Message->m_Des.m_Type == Payload_Address_Type_MAC)
	{
		UInt8 mac[6];
		GetMAC(mac);
		if(CommperMAC(mac, _Message->m_Des.m_Address.MAC) == False)
		{
			return 0;
		}
	}
	else if(_Message->m_Type != Payload_Type_Broadcast && _Message->m_Type != Payload_Type_BroadcastRespons)
	{
		return 0;
	}

	if(_NetworkLayer->m_FuncOut.m_Receive != NULL)
	{
		Payload replay;
		Payload_Initialize(&replay, _Replay->m_UUID);
		
		replay.m_Src.m_Type = Payload_Address_Type_MAC;
		GetMAC(replay.m_Src.m_Address.MAC);
		int success = _NetworkLayer->m_FuncOut.m_Receive(_NetworkLayer->m_FuncOut.m_Context, _Message, &replay);
		if(success == 1)
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
		if(success < 0)
			return success - 3;
	}

	return 0;
}

int NetworkLayer_PayloadBuilder(NetworkLayer* _NetworLayer, Payload* _Payload)
{
	_Payload->m_Src.m_Type = Payload_Address_Type_IP;
	GetIP((UInt8*)&_Payload->m_Src.m_Address);

	return 0;
}


void NetworkLayer_Dispose(NetworkLayer* _NetworkLayer)
{
	

	if(_NetworkLayer->m_Allocated == True)
		Allocator_Free(_NetworkLayer);
	else
		memset(_NetworkLayer, 0, sizeof(NetworkLayer));

}