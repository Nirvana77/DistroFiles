#include "Filesystem_Server.h"

int Filesystem_Server_ConnectedSocket(TCPSocket* _TCPSocket, void* _Context);

int Filesystem_Server_InitializePtr(Filesystem_Service* _Service, Filesystem_Server** _ServerPtr)
{
	Filesystem_Server* _Server = (Filesystem_Server*)Allocator_Malloc(sizeof(Filesystem_Server));
	if(_Server == NULL)
		return -1;
	
	int success = Filesystem_Server_Initialize(_Server, _Service);
	if(success != 0)
	{
		Allocator_Free(_Server);
		return success;
	}
	
	_Server->m_Allocated = True;
	
	*(_ServerPtr) = _Server;
	return 0;
}

int Filesystem_Server_Initialize(Filesystem_Server* _Server, Filesystem_Service* _Service)
{
	_Server->m_Allocated = False;
	_Server->m_Service = _Service;

	int TCPSuccess = TCPServer_Initialize(&_Server->m_TCPServer, Filesystem_Server_ConnectedSocket, _Server);
	if(TCPSuccess != 0)
	{
		printf("TCP Server error: %i\n\r", TCPSuccess);
		return -5;
	}

	TCPSuccess = TCPServer_Listen(&_Server->m_TCPServer, _Server->m_Service->m_Settings.m_IP.m_Ptr, _Server->m_Service->m_Settings.m_Port);
	if(TCPSuccess != 0)
	{
		printf("TCP Listen error: %i\n\r", TCPSuccess);
		return -5;
	}

	Buffer_Initialize(&_Server->m_Buffer, 64);

	return 0;
}

int Filesystem_Server_ConnectedSocket(TCPSocket* _TCPSocket, void* _Context)
{
	Filesystem_Server* _Server = (Filesystem_Server*) _Context;

	LinkedList_Push(&_Server->m_Sockets, _TCPSocket);

	char ip[17];
	memset(ip, 0, sizeof(ip));
	inet_ntop(AF_INET, &_TCPSocket->m_Addr.sin_addr.s_addr, ip, sizeof(ip));
	printf("Connected socket(%u): %s\n\r", _TCPSocket->m_Addr.sin_port, ip);

	return 0;
}

void Filesystem_Server_Work(UInt64 _MSTime, Filesystem_Server* _Server)
{
	TCPServer_Work(&_Server->m_TCPServer);

	Buffer_Clear(&_Server->m_Buffer);
	LinkedList_Node* currentNode = _Server->m_Sockets.m_Head;
	while(currentNode != NULL)
	{
		TCPSocket* TCPSocket = currentNode->m_Item;

		int readBytes = TCPSocket_Read(TCPSocket, &_Server->m_Buffer, 64);

		if(readBytes > 0)
		{
			char str[readBytes + 1];
			Buffer_ReadBuffer(&_Server->m_Buffer, str, readBytes);
			printf("Resived: %s\r\n", str);
		}

		currentNode = currentNode->m_Next;
		Buffer_Clear(&_Server->m_Buffer);
	}

	// printf("Work%lu\n\r", _MSTime);
}

void Filesystem_Server_Dispose(Filesystem_Server* _Server)
{
	TCPServer_Dispose(&_Server->m_TCPServer);

	Buffer_Dispose(&_Server->m_Buffer);

	LinkedList_Node* currentNode = _Server->m_Sockets.m_Head;
	while(currentNode != NULL)
	{
		TCPSocket* TCPSocket = currentNode->m_Item;
		currentNode = currentNode->m_Next;

		TCPSocket_Dispose(TCPSocket);
		LinkedList_RemoveFirst(&_Server->m_Sockets);
	}

	LinkedList_Dispose(&_Server->m_Sockets);

	if(_Server->m_Allocated == True)
		Allocator_Free(_Server);
	else
		memset(_Server, 0, sizeof(Filesystem_Server));

}
