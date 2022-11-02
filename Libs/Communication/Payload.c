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

	_Payload->m_Type = Payload_Type_UnSafe;

	Buffer_Initialize(&_Payload->m_Data, True, 64);

	memset(&_Payload->m_Src, 0, sizeof(Payload_Address));
	memset(&_Payload->m_Des, 0, sizeof(Payload_Address));
	memset(&_Payload->m_Message, 0, sizeof(Payload_Message));
	
	return 0;
}

int Payload_WriteCommunicator(Payload_Address* _Communicator, Buffer* _Buffer)
{

	Buffer_WriteUInt8(_Buffer, _Communicator->m_Type);
	return Buffer_WriteBuffer(_Buffer, (unsigned char*)&_Communicator->m_Address, sizeof(_Communicator->m_Address)) + 1;
}

int Payload_WriteMessage(Payload_Message* _Message, Buffer* _Buffer)
{
	if(_Message->m_Type == Payload_Message_Type_None)
		return 0;

	int success = Buffer_WriteUInt8(_Buffer, _Message->m_Type);

	if(success < 0)
		return -1;
	int written = success;
	
	success = Buffer_WriteUInt16(_Buffer, _Message->m_Size);

	if(success < 0)
		return -2;
	written += success;

	switch (_Message->m_Type)
	{
		case Payload_Message_Type_String:
		{
			
			success = Buffer_WriteBuffer(_Buffer, (UInt8*)_Message->m_Method.m_Str, _Message->m_Size);

			if(written < 0 )
				return -3;
			written += success;

		} break;

		default:
		{

		} break;
	}

	return written;
}

int Payload_ReadCommunicator(Payload_Address* _Communicator, Buffer* _Buffer)
{
	switch (_Communicator->m_Type)
	{
		case Payload_Address_Type_IP:
		{
			return Buffer_ReadBuffer(_Buffer, _Communicator->m_Address.IP, 4);
		} break;
		case Payload_Address_Type_MAC:
		{
			return Buffer_ReadBuffer(_Buffer, _Communicator->m_Address.MAC, 6);
		} break;
		case Payload_Address_Type_NONE:
		{

		} break;
	}

	return 0;
}

int Payload_ReadMessage(Payload_Message* _Message, Buffer* _Buffer)
{
	int success = 0;
	int readed = 0;
	UInt8 type = 0;
	success = Buffer_ReadUInt8(_Buffer, &type);
	if(success < 0)
		return -1;

	readed += success;
	_Message->m_Type = (Payload_Message_Type)type;

	Buffer_ReadUInt16(_Buffer, &_Message->m_Size);
	if(success < 0)
		return -2;

	readed += success;

	switch (_Message->m_Type)
	{
		case Payload_Message_Type_String:
		{
			Buffer_ReadBuffer(_Buffer, (UInt8*)_Message->m_Method.m_Str, _Message->m_Size);
			if(success < 0)
				return -3;

			_Message->m_Method.m_Str[_Message->m_Size] = 0;

			readed += success;

		} break;
		case Payload_Message_Type_None:
		{
		} break;
	}

	return readed;
}

void Payload_FilCommunicator(Payload_Address* _Des, Payload_Address* _Src)
{
	_Des->m_Type = _Src->m_Type;
	switch (_Src->m_Type)
	{
		case Payload_Address_Type_IP:
		{
			_Des->m_Address.IP[0] = _Src->m_Address.IP[0];
			_Des->m_Address.IP[1] = _Src->m_Address.IP[1];
			_Des->m_Address.IP[2] = _Src->m_Address.IP[2];
			_Des->m_Address.IP[3] = _Src->m_Address.IP[3];
		} break;
		case Payload_Address_Type_MAC:
		{
			_Des->m_Address.MAC[0] = _Src->m_Address.MAC[0];
			_Des->m_Address.MAC[1] = _Src->m_Address.MAC[1];
			_Des->m_Address.MAC[2] = _Src->m_Address.MAC[2];
			_Des->m_Address.MAC[3] = _Src->m_Address.MAC[3];
			_Des->m_Address.MAC[4] = _Src->m_Address.MAC[4];
			_Des->m_Address.MAC[5] = _Src->m_Address.MAC[5];

		} break;
		case Payload_Address_Type_NONE:
		{

		} break;
	}
}

void Payload_FilMessage(Payload_Message* _Des, Payload_Message* _Src)
{
	_Des->m_Type = _Src->m_Type;
	_Des->m_Size = _Src->m_Size;

	switch (_Src->m_Type)
	{
		case Payload_Message_Type_String:
		{
			
			strncpy(_Des->m_Method.m_Str, _Src->m_Method.m_Str, strlen(_Src->m_Method.m_Str));
		} break;
		case Payload_Message_Type_None:
		{
			return;
		} break;
	}
}

void Payload_Copy(Payload* _Des, Payload* _Src)
{
	_Des->m_Size = _Src->m_Size;
	_Des->m_Time = _Src->m_Time;
	_Des->m_Type = _Src->m_Type;
	_Des->m_State = _Src->m_State;

	Payload_FilCommunicator(&_Des->m_Des, &_Src->m_Des);
	Payload_FilCommunicator(&_Des->m_Src, &_Src->m_Src);

	Payload_FilMessage(&_Des->m_Message, &_Src->m_Message);

	Buffer_Copy(&_Des->m_Data, &_Src->m_Data, 0);
}

void Payload_Dispose(Payload* _Payload)
{
	Buffer_Dispose(&_Payload->m_Data);

	if(_Payload->m_Allocated == True)
		Allocator_Free(_Payload);
	else
		memset(_Payload, 0, sizeof(Payload));

}