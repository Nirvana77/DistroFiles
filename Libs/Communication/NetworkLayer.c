#include "NetworkLayer.h"

int NetworkLayer_PayloadLinker(NetworkLayer* _NetworLayer, Payload* _Dst, Payload* _Src);

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

int NetworkLayer_SendPayload(void* _Context, Payload* _Paylode)
{
	NetworkLayer* _NetworkLayer = (NetworkLayer*) _Context;
	if(_NetworkLayer->m_FuncOut.m_Send == NULL)
		return 0;


	Payload message;
	Payload_Initialize(&message);

	if(_NetworkLayer->m_FuncOut.m_Send(_NetworkLayer->m_FuncOut.m_Context, &message) == 1) //Whant to send meesage
	{
		printf("NetworkLayer_SendPayload\n\r");
		int success = NetworkLayer_PayloadLinker(_NetworkLayer, _Paylode, &message);
		if(success == 0)
		{
			Payload_Dispose(&message);
			return 1;

		}
	}

	Payload_Dispose(&message);

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

	Payload_ReadMessage(&_Message->m_Message, &_Message->m_Data);

	Buffer_ReadUInt16(&_Message->m_Data, &_Message->m_Size);

	if(_NetworkLayer->m_FuncOut.m_Receive != NULL)
	{
		Payload replay;
		Payload_Initialize(&replay);
		if(_NetworkLayer->m_FuncOut.m_Receive(_NetworkLayer->m_FuncOut.m_Context, _Message, &replay) == 1)
		{//Whant to send replay

			Payload_Dispose(&replay);
			return 1;
		}
		Payload_Dispose(&replay);
	}

	return 0;
}

//TODO #29 make the method a bit at the start
int NetworkLayer_PayloadLinker(NetworkLayer* _NetworLayer, Payload* _Dst, Payload* _Src)
{

	int success = Buffer_WriteUInt8(&_Dst->m_Data, _Src->m_Type);
	if(success < 0)
		return -1;

	success = Buffer_WriteUInt64(&_Dst->m_Data, _Src->m_Time);
	if(success < 0)
		return -2;

	success = Buffer_WriteUInt8(&_Dst->m_Data, _Src->m_Src.m_Type);
	if(success < 0)
		return -3;
	success = Payload_WriteCommunicator(&_Src->m_Src, &_Dst->m_Data);
	if(success < 0)
		return -4;

	success = Buffer_WriteUInt8(&_Dst->m_Data, _Src->m_Des.m_Type);
	if(success < 0)
		return -5;
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