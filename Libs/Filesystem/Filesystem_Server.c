#include "Filesystem_Server.h"

int Filesystem_Server_ConnectedSocket(TCPSocket* _TCPSocket, void* _Context);

int Filesystem_Server_TCPRead(void* _Context, Buffer* _Buffer, int _Size);
int Filesystem_Server_TCPWrite(void* _Context, Buffer* _Buffer, int _Size);

int Filesystem_Server_SendPayload(void* _Context, Payload* _Paylode);
int Filesystem_Server_ReveicePayload(void* _Context, Payload* _Paylode);

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

	int success = TCPServer_Initialize(&_Server->m_TCPServer, Filesystem_Server_ConnectedSocket, _Server);
	if(success != 0)
	{
		printf("Failed to initialize the TCP Server!\n\r");
		printf("Error code: %i\n\r", success);
		return -2;
	}

	success = TCPServer_Listen(&_Server->m_TCPServer, _Server->m_Service->m_Settings.m_IP.m_Ptr, _Server->m_Service->m_Settings.m_Port);
	if(success != 0)
	{
		printf("TCP Listen error: %i\n\r", success);
		TCPServer_Disconnect(&_Server->m_TCPServer);
		return -3;
	}

	success = Buffer_Initialize(&_Server->m_Buffer, 64);
	if(success != 0)
	{
		printf("Failed to initialize the Buffer!\n\r");
		printf("Error code: %i\n\r", success);
		TCPServer_Disconnect(&_Server->m_TCPServer);
		return -4;
	}
	LinkedList_Initialize(&_Server->m_Sockets);


	success = DataLayer_Initialize(&_Server->m_DataLayer, NULL, Filesystem_Server_TCPRead, Filesystem_Server_TCPWrite, NULL, _Server, 100);
	if(success != 0)
	{
		printf("Failed to initialize the DataLayer!\n\r");
		printf("Error code: %i\n\r", success);
		TCPServer_Disconnect(&_Server->m_TCPServer);
		LinkedList_Dispose(&_Server->m_Sockets);
		Buffer_Dispose(&_Server->m_Buffer);
		return -5;
	}

	Payload_FuncOut_Set(&_Server->m_DataLayer.m_FuncOut, Filesystem_Server_ReveicePayload, Filesystem_Server_SendPayload, _Server);


	return 0;
}

int Filesystem_Server_ConnectedSocket(TCPSocket* _TCPSocket, void* _Context)
{
	Filesystem_Server* _Server = (Filesystem_Server*) _Context;

	LinkedList_Push(&_Server->m_Sockets, _TCPSocket);

	if(_Server->m_CurrentNode == NULL)
		_Server->m_CurrentNode = _Server->m_Sockets.m_Head;

	char ip[17];
	memset(ip, 0, sizeof(ip));
	inet_ntop(AF_INET, &_TCPSocket->m_Addr.sin_addr.s_addr, ip, sizeof(ip));
	printf("Connected socket(%u): %s\n\r", _TCPSocket->m_Addr.sin_port, ip);

	return 0;
}


int Filesystem_Server_TCPRead(void* _Context, Buffer* _Buffer, int _Size)
{
	Filesystem_Server* _Server = (Filesystem_Server*) _Context;

	if(_Server->m_CurrentNode == NULL)
		return 0;

	Buffer_Clear(&_Server->m_Buffer);
	LinkedList_Node* currentNode = _Server->m_CurrentNode;
	_Server->m_CurrentNode = _Server->m_CurrentNode->m_Next;

	if(_Server->m_CurrentNode == NULL)
		_Server->m_CurrentNode = _Server->m_Sockets.m_Head;

	TCPSocket* TCPSocket = currentNode->m_Item;

	int readBytes = TCPSocket_Read(TCPSocket, &_Server->m_Buffer, 64);

	if(readBytes > 0)
	{
		char str[readBytes + 1];
		Buffer_ReadBuffer(&_Server->m_Buffer, (UInt8*)str, readBytes);
		printf("Resived(%u): %s\r\n", readBytes, str);
		return readBytes;
	}

	return 0;
}

int Filesystem_Server_TCPWrite(void* _Context, Buffer* _Buffer, int _Size)
{
	Filesystem_Server* _Server = (Filesystem_Server*) _Context;

	printf("Filesystem_Server_TCPWrite\n\r");

	return 0;
}


int Filesystem_Server_SendPayload(void* _Context, Payload* _Paylode)
{
	Filesystem_Server* _Server = (Filesystem_Server*) _Context;

	printf("Filesystem_Server_SendPayload\n\r");

	return 0;
}

int Filesystem_Server_ReveicePayload(void* _Context, Payload* _Paylode)
{
	Filesystem_Server* _Server = (Filesystem_Server*) _Context;

	printf("Filesystem_Server_ReveicePayload\n\r");

	return 0;
}

void Filesystem_Server_Work(UInt64 _MSTime, Filesystem_Server* _Server)
{
	TCPServer_Work(&_Server->m_TCPServer);
	DataLayer_Work(_MSTime, &_Server->m_DataLayer);
/* 
	Buffer_Clear(&_Server->m_Buffer);
	LinkedList_Node* currentNode = _Server->m_Sockets.m_Head;
	while(currentNode != NULL)
	{
		TCPSocket* TCPSocket = currentNode->m_Item;

		int readBytes = TCPSocket_Read(TCPSocket, &_Server->m_Buffer, 64);

		if(readBytes > 0)
		{
			char str[readBytes + 1];
			Buffer_ReadBuffer(&_Server->m_Buffer, (UInt8*)str, readBytes);
			printf("Resived: %s\r\n", str);
		}

		currentNode = currentNode->m_Next;
		Buffer_Clear(&_Server->m_Buffer);
	} */

	// printf("Work%lu\n\r", _MSTime);
}

void Filesystem_Server_Dispose(Filesystem_Server* _Server)
{

	LinkedList_Node* currentNode = _Server->m_Sockets.m_Head;
	while(currentNode != NULL)
	{
		TCPSocket* TCPSocket = currentNode->m_Item;
		currentNode = currentNode->m_Next;

		TCPSocket_Dispose(TCPSocket);
		LinkedList_RemoveFirst(&_Server->m_Sockets);
	}

	DataLayer_Dispose(&_Server->m_DataLayer);
	TCPServer_Dispose(&_Server->m_TCPServer);

	LinkedList_Dispose(&_Server->m_Sockets);
	Buffer_Dispose(&_Server->m_Buffer);

	if(_Server->m_Allocated == True)
		Allocator_Free(_Server);
	else
		memset(_Server, 0, sizeof(Filesystem_Server));

}
