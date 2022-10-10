#include "Filesystem_Client.h"

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
	int success = TCPClient_Initialize(&_Client->m_TCPClient, "127.0.0.1", 5566);

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
		return -2;
	}

	return 0;
}

void Filesystem_Client_Work(UInt64 _MSTime, Filesystem_Client* _Client)
{

}


void Filesystem_Client_Dispose(Filesystem_Client* _Client)
{
	TCPClient_Dispose(&_Client->m_TCPClient);

	if(_Client->m_Allocated == True)
		Allocator_Free(_Client);
	else
		memset(_Client, 0, sizeof(Filesystem_Client));

}