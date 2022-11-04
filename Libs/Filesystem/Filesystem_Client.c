#include "Filesystem_Client.h"

int Filesystem_Client_ConnectedSocket(TCPSocket* _TCPSocket, void* _Context);

int Filesystem_Client_TCPRead(void* _Context, Buffer* _Buffer, int _Size);
int Filesystem_Client_TCPWrite(void* _Context, Buffer* _Buffer, int _Size);

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
	_Client->m_NextCheck = 0;
	_Client->m_Timeout = 10000;
	_Client->m_Service = _Service;
	_Client->m_Server = _Service->m_Server;

	int success = TCPServer_Initialize(&_Client->m_TCPServer, Filesystem_Client_ConnectedSocket, _Client);
	if(success != 0)
	{
		printf("Failed to initialize the TCPServer for Server!\n\r");
		printf("Error code: %i\n\r", success);
		return -2;
	}

	success = TCPServer_Listen(&_Client->m_TCPServer, _Service->m_Settings.m_Distributer.m_IP.m_Ptr, _Service->m_Settings.m_Distributer.m_Port);
	if(success != 0)
	{
		printf("Failed to listen to port %u for server!\n\r", _Client->m_Service->m_Settings.m_Host.m_Port);
		printf("Error code: %i\n\r", success);
		TCPServer_Dispose(&_Client->m_TCPServer);
		return -3;
	}

	success = Buffer_Initialize(&_Client->m_Buffer, True, 64);
	if(success != 0)
	{
		printf("Failed to initialize the Buffer!\n\r");
		printf("Error code: %i\n\r", success);
		TCPServer_Disconnect(&_Client->m_TCPServer);
		return -4;
	}
	LinkedList_Initialize(&_Client->m_Sockets);


	success = DataLayer_Initialize(&_Client->m_DataLayer, NULL, Filesystem_Client_TCPRead, Filesystem_Client_TCPWrite, NULL, _Client, 100);
	if(success != 0)
	{
		printf("Failed to initialize the DataLayer for server!\n\r");
		printf("Error code: %i\n\r", success);
		TCPServer_Disconnect(&_Client->m_TCPServer);
		LinkedList_Dispose(&_Client->m_Sockets);
		Buffer_Dispose(&_Client->m_Buffer);
		return -5;
	}

	success = NetworkLayer_Initialize(&_Client->m_NetworkLayer);
	if(success != 0)
	{
		printf("Failed to initialize the NetworkLayer for server!\n\r");
		printf("Error code: %i\n\r", success);
		TCPServer_Disconnect(&_Client->m_TCPServer);
		LinkedList_Dispose(&_Client->m_Sockets);
		Buffer_Dispose(&_Client->m_Buffer);
		DataLayer_Dispose(&_Client->m_DataLayer);
		return -6;
	}

	success = TransportLayer_Initialize(&_Client->m_TransportLayer);
	if(success != 0)
	{
		printf("Failed to initialize the TransportLayer for server!\n\r");
		printf("Error code: %i\n\r", success);
		TCPServer_Disconnect(&_Client->m_TCPServer);
		LinkedList_Dispose(&_Client->m_Sockets);
		Buffer_Dispose(&_Client->m_Buffer);
		DataLayer_Dispose(&_Client->m_DataLayer);
		NetworkLayer_Dispose(&_Client->m_NetworkLayer);
		return -6;
	}

	Payload_FuncOut_Set(&_Client->m_DataLayer.m_FuncOut, NetworkLayer_ReveicePayload, NetworkLayer_SendPayload, &_Client->m_NetworkLayer);
	Payload_FuncOut_Set(&_Client->m_NetworkLayer.m_FuncOut, TransportLayer_ReveicePayload, TransportLayer_SendPayload, &_Client->m_TransportLayer);
	Payload_FuncOut_Set(&_Client->m_TransportLayer.m_FuncOut, Filesystem_Client_ReveicePayload, NULL, _Client);

	return 0;
}

int Filesystem_Client_ConnectedSocket(TCPSocket* _TCPSocket, void* _Context)
{
	Filesystem_Client* _Client = (Filesystem_Client*) _Context;

	char ip[17];
	memset(ip, 0, sizeof(ip));
	inet_ntop(AF_INET, &_TCPSocket->m_Addr.sin_addr.s_addr, ip, sizeof(ip));
	
	printf("Filesystem_Client: Connected socket(%u): %s\n\r", (unsigned int)ntohs(_TCPSocket->m_Addr.sin_port), ip);

	LinkedList_Push(&_Client->m_Sockets, _TCPSocket);
	return 0;
	
	/*
	* NOTE: you dont get any information abute the sender
	* Just the socket
	*/
	/*
	Buffer data;
	Buffer_Initialize(&data, True, 16);

	Buffer_WriteUInt8(&data, Payload_Type_Safe);
	UInt64 time = 0;
	SystemMonotonicMS(&time);
	Buffer_WriteUInt64(&data, time);
	
	Buffer_WriteUInt8(&data, Payload_Address_Type_MAC);
	UInt8 mac[6];
	GetMAC(mac);
	Buffer_WriteBuffer(&data, mac, 6);
	Buffer_WriteUInt8(&data, Payload_Address_Type_NONE);

	Buffer_WriteUInt8(&data, Payload_Message_Type_String);
	UInt16 size = strlen("Connect");
	Buffer_WriteUInt16(&data, size);
	Buffer_WriteBuffer(&data, "Connect", size);

	Buffer_WriteUInt16(&data, 0);
	
	UInt8 CRC = 0;
	DataLayer_GetCRC(data.m_Ptr, data.m_BytesLeft, &CRC);
	Buffer_WriteUInt8(&data, CRC);

	TCPSocket_Write(_TCPSocket, &data, data.m_BytesLeft);

	Buffer_Dispose(&data);
	return 1;
	*/
}


int Filesystem_Client_TCPRead(void* _Context, Buffer* _Buffer, int _Size)
{
	Filesystem_Client* _Client = (Filesystem_Client*) _Context;

	if(_Client->m_Sockets.m_Size == 0)
		return 0;

	int readed = 0;
	LinkedList_Node* currentNode = _Client->m_Sockets.m_Head;

	Buffer_Clear(&_Client->m_Buffer);
	while(currentNode != NULL)
	{	
		TCPSocket* socket = (TCPSocket*)currentNode->m_Item;

		readed += TCPSocket_Read(socket, &_Client->m_Buffer, 1024);

		currentNode = currentNode->m_Next;
	}

	if(readed > 0)
	{
		printf("Filesystem_Client_TCPRead\n\r");
		Buffer_Copy(_Buffer, &_Client->m_Buffer, _Client->m_Buffer.m_BytesLeft);
		return readed;
	}

	return 0;

}

int Filesystem_Client_TCPWrite(void* _Context, Buffer* _Buffer, int _Size)
{
	Filesystem_Client* _Client = (Filesystem_Client*) _Context;

	printf("Filesystem_Client_TCPWrite\n\r");

	LinkedList_Node* currentNode = _Client->m_Sockets.m_Head;
	while (currentNode != NULL)
	{
		TCPSocket* socket = (TCPSocket*) currentNode->m_Item;
		Buffer_ResetReadPtr(_Buffer);
		TCPSocket_Write(socket, _Buffer, _Size);

		currentNode = currentNode->m_Next;
	}

	return 0;
}

int Filesystem_Client_ReveicePayload(void* _Context, Payload* _Message, Payload* _Replay)
{
	Filesystem_Client* _Client = (Filesystem_Client*) _Context;

	printf("Filesystem_Client_ReveicePayload(%i)\n\r", _Message->m_Message.m_Type);
	if(_Message->m_Message.m_Type != Payload_Message_Type_String)
		return 0;

	printf("Method: %s\n\r", _Message->m_Message.m_Method.m_Str);

	if(strcmp(_Message->m_Message.m_Method.m_Str, "upload") == 0)
	{
		Bool isFile = True;
		UInt16 size = 0;

		Buffer_ReadUInt8(&_Message->m_Data, (UInt8*)&isFile);
		Buffer_ReadUInt16(&_Message->m_Data, &size);

		char name[size + 1];
		Buffer_ReadBuffer(&_Message->m_Data, (unsigned char*)name, size);
		name[size] = 0;

		Filesystem_Server_Write(_Client->m_Server, isFile, name, &_Message->m_Data);
		
	}
	else if(strcmp(_Message->m_Message.m_Method.m_Str, "list") == 0)
	{
		UInt16 size = 0;
		Buffer_ReadUInt16(&_Message->m_Data, &size);

		char path[size + 1];
		Buffer_ReadBuffer(&_Message->m_Data, (unsigned char*)path, size);
		path[size] = 0;

		_Replay->m_Size += Filesystem_Server_GetList(_Client->m_Server, path, &_Replay->m_Data);
		Payload_SetMessageType(_Replay, Payload_Message_Type_String, "listRespons", strlen("listRespons"));
		return 1;
	}
	else
	{
		printf("Server can't handel method: %s\n\r", _Message->m_Message.m_Method.m_Str);
	}

	return 0;
}

void Filesystem_Client_Work(UInt64 _MSTime, Filesystem_Client* _Client)
{
	TCPServer_Work(&_Client->m_TCPServer);
	DataLayer_Work(_MSTime, &_Client->m_DataLayer);
	TransportLayer_Work(_MSTime, &_Client->m_TransportLayer);
	
	if(_MSTime > _Client->m_NextCheck)
	{
		_Client->m_NextCheck = _MSTime + _Client->m_Timeout;
		LinkedList_Node* currentNode = _Client->m_Sockets.m_Head;
		while (currentNode != NULL)
		{
			TCPSocket* socket = (TCPSocket*)currentNode->m_Item;
			currentNode = currentNode->m_Next;

			int succeess = recv(socket->m_FD,NULL,1, MSG_PEEK | MSG_DONTWAIT);

			if(succeess == 0)
			{
				LinkedList_RemoveItem(&_Client->m_Sockets, socket);
				TCPSocket_Dispose(socket);
			}
		}
		
	}

}

void Filesystem_Client_Dispose(Filesystem_Client* _Client)
{
	TransportLayer_Dispose(&_Client->m_TransportLayer);
	NetworkLayer_Dispose(&_Client->m_NetworkLayer);
	DataLayer_Dispose(&_Client->m_DataLayer);

	LinkedList_Node* currentNode = _Client->m_Sockets.m_Head;
	while(currentNode != NULL)
	{
		TCPSocket* TCPSocket = currentNode->m_Item;
		currentNode = currentNode->m_Next;

		TCPSocket_Dispose(TCPSocket);
		LinkedList_RemoveFirst(&_Client->m_Sockets);
	}

	TCPServer_Dispose(&_Client->m_TCPServer);

	LinkedList_Dispose(&_Client->m_Sockets);
	Buffer_Dispose(&_Client->m_Buffer);

	if(_Client->m_Allocated == True)
		Allocator_Free(_Client);
	else
		memset(_Client, 0, sizeof(Filesystem_Client));

}