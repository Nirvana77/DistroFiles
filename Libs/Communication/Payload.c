#include "Payload.h"

int Payload_InitializePtr(Payload** _PayloadPtr)
{
	Payload* _Payload = (Payload*)Allocator_Malloc(sizeof(Payload));
	if(_Payload == NULL)
		return -1;
	
	int success = Payload_Initialize(_Payload);
	if(success != 0)
	{
		Allocator_Free(_Payload);
		return success;
	}
	
	_Payload->m_Allocated = True;
	
	*(_PayloadPtr) = _Payload;
	return 0;
}

int Payload_Initialize(Payload* _Payload)
{
	_Payload->m_Allocated = False;
	_Payload->m_State = Payload_State_Init;

	_Payload->m_Size = 0;
	_Payload->m_Time = 0;

	_Payload->m_Type = Payload_MessageType_UnSafe;

	Buffer_Initialize(&_Payload->m_Data, True, 16);
	
	return 0;
}

int Payload_WriteCommunicator(Payload_Communicator* _Communicator, Buffer* _Buffer)
{
	switch (_Communicator->m_Type)
	{
		case Payload_Communicator_Type_IP:
		{
			return Buffer_WriteBuffer(_Buffer, _Communicator->m_Address.IP, 4);
		} break;
		case Payload_Communicator_Type_MAC:
		{
			return Buffer_WriteBuffer(_Buffer, _Communicator->m_Address.MAC, 6);
		} break;
	}

	return 0;
}

int Payload_ReadCommunicator(Payload_Communicator* _Communicator, Buffer* _Buffer)
{
	switch (_Communicator->m_Type)
	{
		case Payload_Communicator_Type_IP:
		{
			return Buffer_ReadBuffer(_Buffer, _Communicator->m_Address.IP, 4);
		} break;
		case Payload_Communicator_Type_MAC:
		{
			return Buffer_ReadBuffer(_Buffer, _Communicator->m_Address.MAC, 6);
		} break;
	}

	return 0;
}


void Payload_Dispose(Payload* _Payload)
{
	Buffer_Dispose(&_Payload->m_Data);

	if(_Payload->m_Allocated == True)
		Allocator_Free(_Payload);
	else
		memset(_Payload, 0, sizeof(Payload));

}