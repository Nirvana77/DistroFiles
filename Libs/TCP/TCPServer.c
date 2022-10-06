#include "TCPServer.h"

int TCPServer_InitializePtr(const char* _IP, int _Port, TCPServer** _TCPServerPtr)
{
	TCPServer* _TCPServer = (TCPServer*)Allocator_Malloc(sizeof(TCPServer));
	if(_TCPServer == NULL)
		return -1;
	
	int success = TCPServer_Initialize(_TCPServer, _IP, _Port);
	if(success != 0)
	{
		Allocator_Free(_TCPServer);
		return success;
	}
	
	_TCPServer->m_Allocated = True;
	
	*(_TCPServerPtr) = _TCPServer;
	return 0;
}

int TCPServer_Initialize(TCPServer* _TCPServer, const char* _IP, int _Port)
{
	_TCPServer->m_Allocated = False;
	_TCPServer->m_Port = _Port;

	if(TCPSocket_Initialize(&_TCPServer->m_Socket, _IP, _Port) != 0);
	{
		return -2;
	}



	return 0;
}

int TCPServer_Listen(TCPServer* _TCPServer)
{
	return 0;
}

void TCPServer_Disconnect(TCPServer* _TCPServer)
{
	
}

void TCPServer_Dispose(TCPServer* _TCPServer)
{
	TCPServer_Disconnect(_TCPServer);

	if(_TCPServer->m_Allocated == True)
		Allocator_Free(_TCPServer);
	else
		memset(_TCPServer, 0, sizeof(TCPServer));

}