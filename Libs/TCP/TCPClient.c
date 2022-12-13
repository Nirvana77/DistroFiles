#include "TCPClient.h"

int TCPClient_InitializePtr(const char* _Host, UInt16 _Port, TCPClient** _TCPClientPtr)
{
	TCPClient* _TCPClient = (TCPClient*)Allocator_Malloc(sizeof(TCPClient));
	if(_TCPClient == NULL)
		return -1;
	
	int success = TCPClient_Initialize(_TCPClient, _Host, _Port);
	if(success != 0)
	{
		Allocator_Free(_TCPClient);
		return success;
	}
	
	_TCPClient->m_Allocated = True;
	
	*(_TCPClientPtr) = _TCPClient;
	return 0;
}

int TCPClient_Initialize(TCPClient* _TCPClient, const char* _Host, UInt16 _Port)
{
	_TCPClient->m_Allocated = False;
	_TCPClient->m_Socket = NULL;

	if(String_Initialize(&_TCPClient->m_Host, 16) != 0)
		return -2;

	if(String_Set(&_TCPClient->m_Host, _Host) != 0)
		return -3;

	_TCPClient->m_Port = _Port;
	_TCPClient->m_State = TCPSocket_Status_Init;
	
	return 0;
}

int TCPClient_Connect(TCPClient* _TCPClient)
{
	printf("TCPClient_Connect(%u): %s\n\r", _TCPClient->m_Port, _TCPClient->m_Host.m_Ptr);

	int success = TCPSocket_InitializePtr(_TCPClient->m_Host.m_Ptr, _TCPClient->m_Port, NULL, &_TCPClient->m_Socket);
	if(success != 0)
	{
		printf("TCPSocket_InitializePtr: %i\n\r", success);
		return -1;
	}

	_TCPClient->m_State = TCPClient_State_Connected;
	return 0;
}

int TCPClient_Read(void* _Context, Buffer* _Buffer)
{
	TCPClient* _TCPClient = (TCPClient*) _Context;
	if(_TCPClient->m_Socket == NULL)
		return -1;

	return TCPSocket_Read(_TCPClient->m_Socket, _Buffer, NULL);
}

int TCPClient_Write(void* _Context, Buffer* _Buffer)
{
	TCPClient* _TCPClient = (TCPClient*) _Context;


	if(_TCPClient->m_Socket == NULL)
		return -1;

	return TCPSocket_Write(_TCPClient->m_Socket, _Buffer, NULL);
}

void TCPClient_Disconnect(TCPClient* _TCPClient)
{
	printf("TCPClient_Disconnect(%u): %s\n\r", _TCPClient->m_Port, _TCPClient->m_Host.m_Ptr);

	if(_TCPClient->m_Socket != NULL)
		TCPSocket_Dispose(_TCPClient->m_Socket);

	_TCPClient->m_State = TCPClient_State_Disconnected;
}

void TCPClient_Dispose(TCPClient* _TCPClient)
{
	TCPClient_Disconnect(_TCPClient);

	String_Dispose(&_TCPClient->m_Host);

	if(_TCPClient->m_Allocated == True)
		Allocator_Free(_TCPClient);
	else
		memset(_TCPClient, 0, sizeof(TCPClient));

}