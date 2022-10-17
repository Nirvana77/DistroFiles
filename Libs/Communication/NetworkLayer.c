#include "NetworkLayer.h"

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

int NetworkLayer_ReveicePayload(void* _Context, Payload* _Paylode)
{
	NetworkLayer* _NetworkLayer = (NetworkLayer*) _Context;

	printf("NetworkLayer_ReveicePayload\n\r");
	
	Buffer_ReadUInt8(&_Paylode->m_Data, (UInt8*)&_Paylode->m_Type);

	Buffer_ReadUInt64(&_Paylode->m_Data, &_Paylode->m_Time);

	Buffer_ReadUInt8(&_Paylode->m_Data, (UInt8*)&_Paylode->m_Src.m_Type);
	Payload_ReadCommunicator(&_Paylode->m_Src, &_Paylode->m_Data);

	Buffer_ReadUInt8(&_Paylode->m_Data, (UInt8*)&_Paylode->m_Des.m_Type);
	Payload_ReadCommunicator(&_Paylode->m_Des, &_Paylode->m_Data);

	Buffer_ReadUInt16(&_Paylode->m_Data, &_Paylode->m_Size);

	int reviced = _NetworkLayer->m_FuncOut.m_Receive(_NetworkLayer->m_FuncOut.m_Context, _Paylode);

	return 0;
}

//TODO make this function
int NetworkLayer_PayloadLinker(NetworkLayer* _NetworLayer, Payload* _Dst, Payload* _Src)
{
	
	/* Buffer_WriteUInt8(&_Paylode->m_Data, _Paylode->m_Type);

	Buffer_WriteUInt64(&_Paylode->m_Data, _Paylode->m_Time);

	Buffer_WriteUInt8(&_Paylode->m_Data, _Paylode->m_Src.m_Type);
	Payload_WriteCommunicator(&_Paylode->m_Src, &_Paylode->m_Data);

	Buffer_WriteUInt8(&_Paylode->m_Data, _Paylode->m_Des.m_Type);
	Payload_WriteCommunicator(&_Paylode->m_Des, &_Paylode->m_Data);
	
	Buffer_WriteUInt16(&_Paylode->m_Data, _Paylode->m_Size);
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