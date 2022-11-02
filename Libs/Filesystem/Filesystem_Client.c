#include "Filesystem_Client.h"
int Filesystem_Client_OnRead(void* _Context, Buffer* _Buffer, int _Size);
int Filesystem_Client_OnWrite(void* _Context, Buffer* _Buffer, int _Size);

int Filesystem_Client_SendPayload(void* _Context, Payload* _Paylode);
int Filesystem_Client_ReveicePayload(void* _Context, Payload* _Message, Payload* _Replay);

int Filesystem_Client_InitializePtr(Filesystem_Service* _Service, Filesystem_Client** _ClientPtr)
{
	Filesystem_Client* _Client = (Filesystem_Client*)Allocator_Malloc(sizeof(Filesystem_Client));
	if(_Client == NULL)
		return -1;
	
	int success = Filesystem_Client_Initialize(_Client, _Service);
	if(success != 0)
	{
		Allocator_Free(_Client);
		return success;
	}
	
	_Client->m_Allocated = True;
	
	*(_ClientPtr) = _Client;
	return 0;
}

int Filesystem_Client_Initialize(Filesystem_Client* _Client, Filesystem_Service* _Service)
{
	_Client->m_Allocated = False;
	_Client->m_Service = _Service;

	//TODO: Change this do not be hard coded
	int success = TCPClient_Initialize(&_Client->m_TCPClient, _Client->m_Service->m_Settings.m_Distributer.m_IP.m_Ptr, _Client->m_Service->m_Settings.m_Distributer.m_Port);

	if(success != 0)
	{
		printf("Failed to initialize the TCP client!\n\r");
		printf("Error code: %i\n\r", success);
		return -2;
	}

	success = TCPClient_Connect(&_Client->m_TCPClient);
	if(success != 0)
	{
		printf("Failed to connect the TCP client!\n\r");
		printf("Error code: %i\n\r", success);
		TCPClient_Dispose(&_Client->m_TCPClient);
		return -3;
	}

	success = DataLayer_Initialize(&_Client->m_DataLayer, NULL, TCPClient_Read, TCPClient_Write, NULL, &_Client->m_TCPClient, 100);
	if(success != 0)
	{
		printf("Failed to initialize the DataLayer for client!\n\r");
		printf("Error code: %i\n\r", success);
		TCPClient_Dispose(&_Client->m_TCPClient);
		return -5;
	}

	success = NetworkLayer_Initialize(&_Client->m_NetworkLayer);
	if(success != 0)
	{
		printf("Failed to initialize the NetworkLayer for client!\n\r");
		printf("Error code: %i\n\r", success);
		TCPClient_Dispose(&_Client->m_TCPClient);
		DataLayer_Dispose(&_Client->m_DataLayer);
		return -6;
	}

	success = TransportLayer_Initialize(&_Client->m_TransportLayer);
	if(success != 0)
	{
		printf("Failed to initialize the TransportLayer for client!\n\r");
		printf("Error code: %i\n\r", success);
		TCPClient_Dispose(&_Client->m_TCPClient);
		DataLayer_Dispose(&_Client->m_DataLayer);
		NetworkLayer_Dispose(&_Client->m_NetworkLayer);
		return -6;
	}

	Payload_FuncOut_Set(&_Client->m_DataLayer.m_FuncOut, NetworkLayer_ReveicePayload, NetworkLayer_SendPayload, &_Client->m_NetworkLayer);
	Payload_FuncOut_Set(&_Client->m_NetworkLayer.m_FuncOut, TransportLayer_ReveicePayload, TransportLayer_SendPayload, &_Client->m_TransportLayer);
	Payload_FuncOut_Set(&_Client->m_TransportLayer.m_FuncOut, Filesystem_Client_ReveicePayload, NULL, _Client);

	return 0;
}

int Filesystem_Client_SendPayload(void* _Context, Payload* _Paylode)
{
	// Filesystem_Client* _Client = (Filesystem_Client*) _Context;
	printf("Filesystem_Client_SendPayload\n\r");
	

	return 0;
}

int Filesystem_Client_ReveicePayload(void* _Context, Payload* _Message, Payload* _Replay)
{
	// Filesystem_Client* _Client = (Filesystem_Client*) _Context;

	printf("Filesystem_Client_ReveicePayload(%i)\n\r", _Message->m_Message.m_Type);
	if(_Message->m_Message.m_Type != Payload_Message_Type_String)
		return 0;

	printf("Method: %s\n\r", _Message->m_Message.m_Method.m_Str);

	if(strcmp(_Message->m_Message.m_Method.m_Str, "Connect") == 0)
	{
		_Replay->m_Type = Payload_Type_Respons;

		printf("Client\n\r");

		_Replay->m_Size = _Replay->m_Data.m_BytesLeft;

		return 1;
	}

	return 0;
}

int Filesystem_Client_SendMessage(Filesystem_Client* _Client, unsigned char* _Data, int _Size)
{
	Payload* _Payload = NULL;
	int success = TransportLayer_CreateMessage(&_Client->m_TransportLayer, Payload_Type_Broadcast, _Size, 1000, &_Payload);
	if(success != 0)
	{
		Payload_Dispose(_Payload);
		return -1;
	}

	Buffer_WriteBuffer(&_Payload->m_Data, _Data, _Size);
	
	return 0;
}


void Filesystem_Client_Work(UInt64 _MSTime, Filesystem_Client* _Client)
{
	DataLayer_Work(_MSTime, &_Client->m_DataLayer);
	TransportLayer_Work(_MSTime, &_Client->m_TransportLayer);
}


void Filesystem_Client_Dispose(Filesystem_Client* _Client)
{
	TransportLayer_Dispose(&_Client->m_TransportLayer);
	NetworkLayer_Dispose(&_Client->m_NetworkLayer);
	DataLayer_Dispose(&_Client->m_DataLayer);

	TCPClient_Dispose(&_Client->m_TCPClient);

	if(_Client->m_Allocated == True)
		Allocator_Free(_Client);
	else
		memset(_Client, 0, sizeof(Filesystem_Client));

}