#include "TCPServer.h"

int TCPServer_InitializePtr(int (*m_ConnectedSocketClackkback)(TCPSocket* _TCPSocket, void* _Context), void* _Context, TCPServer** _TCPServerPtr)
{
	TCPServer* _TCPServer = (TCPServer*)Allocator_Malloc(sizeof(TCPServer));
	if(_TCPServer == NULL)
		return -1;
	
	int success = TCPServer_Initialize(_TCPServer, m_ConnectedSocketClackkback, _Context);
	if(success != 0)
	{
		Allocator_Free(_TCPServer);
		return success;
	}
	
	_TCPServer->m_Allocated = True;
	
	*(_TCPServerPtr) = _TCPServer;
	return 0;
}

int TCPServer_Initialize(TCPServer* _TCPServer, int (*_ConnectedSocketClackkback)(TCPSocket* _TCPSocket, void* _Context), void* _Context)
{
	_TCPServer->m_Allocated = False;

	if(_ConnectedSocketClackkback == NULL)
		return -3;

	_TCPServer->m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(_TCPServer->m_Socket < 0)
	{
		printf("Socket Error: %i\n\r", _TCPServer->m_Socket);
		return -2;
	}
	
	_TCPServer->m_ConnectedSocketClackkback = _ConnectedSocketClackkback;
	_TCPServer->m_Context = _Context;

	TCPSocket_SetNonBlocking(_TCPServer->m_Socket);

	return 0;
}

void TCPServer_Work(TCPServer* _TCPServer)
{
	
	#ifdef __linux__
		socklen_t addr_size = sizeof(addr_size);
		struct sockaddr_in client_addr;
		TCPSocket_FD client_sock = accept(_TCPServer->m_Socket, (struct sockaddr*)&client_addr, &addr_size);
	#else

		int  addr_size = sizeof(addr_size);
		struct sockaddr_in client_addr;
		int client_sock = accept(_TCPServer->m_Socket, (struct sockaddr*)&client_addr, &addr_size);
	#endif
	
	if(client_sock != TCPSocket_Error)
	{
		TCPSocket* newSocket;
		int result = TCPSocket_InitializePtr(0, 0, &client_sock, &newSocket);
		if(result != 0)
		{
			printf("TCPSocket_InitializePtr: %i\n\r", result);
			TCPServer_Disconnect(_TCPServer);
		}
		else
		{
			newSocket->m_Addr = client_addr;
			newSocket->m_Status = TCPSocket_Status_Connected;

			char ip[17];
			memset(ip, 0, sizeof(ip));
			inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, ip, sizeof(ip));
			printf("TCPServer_Work(%u): %s\n\r", client_addr.sin_port, ip);
			if(_TCPServer->m_ConnectedSocketClackkback(newSocket, _TCPServer->m_Context) != 0)
				TCPSocket_Dispose(newSocket);
		}
	}
}

int TCPServer_Listen(TCPServer* _TCPServer, const char* _IP, UInt16 _Port)
{
	printf("TCPServer_Listen(%u): %s\n\r", _Port, _IP);
	memset(&_TCPServer->m_ServerAddr, 0, sizeof(_TCPServer->m_ServerAddr));
	_TCPServer->m_ServerAddr.sin_family = AF_INET;
	_TCPServer->m_ServerAddr.sin_port = _Port;
	_TCPServer->m_ServerAddr.sin_addr.s_addr = inet_addr(_IP);

	int success = bind(_TCPServer->m_Socket, (struct sockaddr*)&_TCPServer->m_ServerAddr, sizeof(_TCPServer->m_ServerAddr));
	if(success != 0)
	{
		TCPServer_Disconnect(_TCPServer);

		return -1;
	}

	success = listen(_TCPServer->m_Socket, SOMAXCONN);
	if(success != 0)
	{
		TCPServer_Disconnect(_TCPServer);

		return -2;
	}

	return 0;
}

void TCPServer_Disconnect(TCPServer* _TCPServer)
{
	char ip[17];
	memset(ip, 0, sizeof(ip));
	inet_ntop(AF_INET, &_TCPServer->m_ServerAddr.sin_addr.s_addr, ip, sizeof(ip));
	printf("TCPServer_Disconnect(%u): %s\n\r", _TCPServer->m_ServerAddr.sin_port, ip);
	#ifdef __linux__
		close(_TCPServer->m_Socket);
	#else

	#endif
}

void TCPServer_Dispose(TCPServer* _TCPServer)
{
	TCPServer_Disconnect(_TCPServer);

	if(_TCPServer->m_Allocated == True)
		Allocator_Free(_TCPServer);
	else
		memset(_TCPServer, 0, sizeof(TCPServer));

}