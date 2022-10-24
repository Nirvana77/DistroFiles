#include "Filesystem_Server.h"

int Filesystem_Server_ConnectedSocket(TCPSocket* _TCPSocket, void* _Context);

int Filesystem_Server_TCPRead(void* _Context, Buffer* _Buffer, int _Size);
int Filesystem_Server_TCPWrite(void* _Context, Buffer* _Buffer, int _Size);
int Filesystem_Server_LoadServer(Filesystem_Server* _Server);

int Filesystem_Server_ReveicePayload(void* _Context, Payload* _Message, Payload* _Replay);

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

	success = TCPServer_Listen(&_Server->m_TCPServer, _Service->m_Settings.m_Host.m_IP.m_Ptr, _Service->m_Settings.m_Host.m_Port);
	if(success != 0)
	{
		printf("TCP Listen error: %i\n\r", success);
		TCPServer_Disconnect(&_Server->m_TCPServer);
		return -3;
	}

	success = Buffer_Initialize(&_Server->m_Buffer, False, 64);
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
		printf("Failed to initialize the DataLayer for server!\n\r");
		printf("Error code: %i\n\r", success);
		TCPServer_Disconnect(&_Server->m_TCPServer);
		LinkedList_Dispose(&_Server->m_Sockets);
		Buffer_Dispose(&_Server->m_Buffer);
		return -5;
	}

	success = NetworkLayer_Initialize(&_Server->m_NetworkLayer);
	if(success != 0)
	{
		printf("Failed to initialize the NetworkLayer for server!\n\r");
		printf("Error code: %i\n\r", success);
		TCPServer_Disconnect(&_Server->m_TCPServer);
		LinkedList_Dispose(&_Server->m_Sockets);
		Buffer_Dispose(&_Server->m_Buffer);
		DataLayer_Dispose(&_Server->m_DataLayer);
		return -6;
	}

	success = TransportLayer_Initialize(&_Server->m_TransportLayer);
	if(success != 0)
	{
		printf("Failed to initialize the TransportLayer for server!\n\r");
		printf("Error code: %i\n\r", success);
		TCPServer_Disconnect(&_Server->m_TCPServer);
		LinkedList_Dispose(&_Server->m_Sockets);
		Buffer_Dispose(&_Server->m_Buffer);
		DataLayer_Dispose(&_Server->m_DataLayer);
		NetworkLayer_Dispose(&_Server->m_NetworkLayer);
		return -6;
	}

	Payload_FuncOut_Set(&_Server->m_DataLayer.m_FuncOut, NetworkLayer_ReveicePayload, NetworkLayer_SendPayload, &_Server->m_NetworkLayer);
	Payload_FuncOut_Set(&_Server->m_NetworkLayer.m_FuncOut, TransportLayer_ReveicePayload, TransportLayer_SendPayload, &_Server->m_TransportLayer);
	Payload_FuncOut_Set(&_Server->m_TransportLayer.m_FuncOut, Filesystem_Server_ReveicePayload, NULL, _Server);

	Filesystem_Server_LoadServer(_Server);

	return 0;
}

int Filesystem_Server_ConnectedSocket(TCPSocket* _TCPSocket, void* _Context)
{
	Filesystem_Server* _Server = (Filesystem_Server*) _Context;

	if(_Server->m_CurrentNode == NULL)
		_Server->m_CurrentNode = _Server->m_Sockets.m_Head;

	char ip[17];
	memset(ip, 0, sizeof(ip));
	inet_ntop(AF_INET, &_TCPSocket->m_Addr.sin_addr.s_addr, ip, sizeof(ip));
	
	printf("Connected socket(%u): %s\n\r", (unsigned int)ntohs(_TCPSocket->m_Addr.sin_port), ip);
	
	/*
	* NOTE: you dont get any information abute the sender
	* Just the socket
	*/
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
}


int Filesystem_Server_TCPRead(void* _Context, Buffer* _Buffer, int _Size)
{
	Filesystem_Server* _Server = (Filesystem_Server*) _Context;

	if(_Server->m_CurrentNode == NULL)
		return 0;

	LinkedList_Node* currentNode = _Server->m_CurrentNode;
	_Server->m_CurrentNode = _Server->m_CurrentNode->m_Next;

	if(_Server->m_CurrentNode == NULL)
		_Server->m_CurrentNode = _Server->m_Sockets.m_Head;

	TCPSocket* TCPSocket = currentNode->m_Item;

	int readBytes = TCPSocket_Read(TCPSocket, _Buffer, _Size);

	if(readBytes > 0)
	{

		printf("Filesystem_Server_TCPRead: %u\r\n", readBytes);
		
		return readBytes;
	}
	return 0;

}

int Filesystem_Server_TCPWrite(void* _Context, Buffer* _Buffer, int _Size)
{
	Filesystem_Server* _Server = (Filesystem_Server*) _Context;

	printf("Filesystem_Server_TCPWrite\n\r");

	LinkedList_Node* currentNode = _Server->m_Sockets.m_Head;
	while (currentNode != NULL)
	{
		TCPSocket* socket = (TCPSocket*) currentNode->m_Item;

		TCPSocket_Write(socket, _Buffer, _Size);

		currentNode = currentNode->m_Next;
	}

	return 0;
}

int Filesystem_Server_LoadServer(Filesystem_Server* _Server)
{
	printf("Filesystem_Server_LoadServer\n\r");
	json_t* members = _Server->m_Service->m_Settings.m_Servers;
	for (int i = 0; i < json_array_size(members); i++)
	{
		json_t* data;
		data = json_array_get(members, i);

		const char* charVal;
		UInt16 port = 0;

		json_getString(data, "IP", &charVal);
		json_getUInt16(data, "port", &port);

		TCPSocket* socket = NULL;
		if(TCPSocket_InitializePtr(charVal, port, NULL, &socket) == 0)
		{
			LinkedList_Push(&_Server->m_Sockets, socket);
		}
		
	}
	
	return 0;
}

int Filesystem_Server_ReveicePayload(void* _Context, Payload* _Message, Payload* _Replay)
{
	Filesystem_Server* _Server = (Filesystem_Server*) _Context;

	printf("Filesystem_Server_ReveicePayload(%i)\n\r", _Message->m_Message.m_Type);
	if(_Message->m_Message.m_Type != Payload_Message_Type_String)
		return 0;

	printf("Method: %s\n\r", _Message->m_Message.m_Method.m_Str);

	if(strcmp(_Message->m_Message.m_Method.m_Str, "SyncAck") == 0)
	{
		unsigned char hash[16];
		unsigned char messageHash[16];

		Folder_Hash(_Server->m_Service->m_FilesytemPath.m_Ptr, hash);
		Buffer_ReadBuffer(&_Message->m_Data, messageHash, 16);

		_Replay->m_Type = Payload_Type_BroadcastRespons;

		if(Filesystem_Server_HashCheck(hash, messageHash) == False)
		{
			
		}
		
	}
	else if(strcmp(_Message->m_Message.m_Method.m_Str, "Sync") == 0)
	{
		
		unsigned char hash[16];
		Folder_Hash(_Server->m_Service->m_FilesytemPath.m_Ptr, hash);
		Buffer_WriteBuffer(&_Replay->m_Data, hash, 16);

		struct stat attr;
		char str[64];
		UInt64 value = 0;
		stat(_Server->m_Service->m_FilesytemPath.m_Ptr, &attr);
		sprintf(str, "%u", attr.st_mtim);

		for (int i = 0; i < strlen(str); i++)
		{
			value += str[i] - 48;
			value *= 10;
		}
		value /= 10;

		Buffer_WriteUInt64(&_Replay->m_Data, value);

		Payload_SetMessageType(_Replay, Payload_Message_Type_String, "SyncAck", strlen("SyncAck"));

		return 1;
	}
	else if(strcmp(_Message->m_Message.m_Method.m_Str, "Update") == 0 ||
			strcmp(_Message->m_Message.m_Method.m_Str, "Create") == 0)
	{
		UInt16 size;
		Buffer_ReadUInt16(&_Message->m_Data, &size);

		char path[size];
		String fullPath;

		String_Initialize(&fullPath, 64);
		String_Set(&fullPath, _Server->m_Service->m_FilesytemPath.m_Ptr);

		if(String_EndsWith(&fullPath, "/") == False)
			String_Append(&fullPath, "/", 1);

		Buffer_ReadBuffer(&_Message->m_Data, (UInt8*)path, size);

		String_Append(&fullPath, path, size);

		//* This can be improved
		Buffer_ReadUInt16(&_Message->m_Data, &size);

		UInt8 data[size];
		Buffer_ReadBuffer(&_Message->m_Data, data, size);
	
		FILE* f = NULL;
		File_Open(fullPath.m_Ptr, "wb+", &f);

		printf("Testing\n\r");
		printf("Path: %s\n\r", path);
		printf("Fullpath: %s\n\r", fullPath.m_Ptr);
		//File_WriteAll(f, data, size);

		File_Close(f);

		String_Dispose(&fullPath);
		return 0;
	}
	else if(strcmp(_Message->m_Message.m_Method.m_Str, "Delete") == 0)
	{

	}
	else if(strcmp(_Message->m_Message.m_Method.m_Str, "Move") == 0 ||
			strcmp(_Message->m_Message.m_Method.m_Str, "Rename"))
	{

	}
	else
	{
		printf("Can't handel method: %s\n\r", _Message->m_Message.m_Method.m_Str);
	}

	return 0;
}

void Filesystem_Server_Work(UInt64 _MSTime, Filesystem_Server* _Server)
{
	TCPServer_Work(&_Server->m_TCPServer);
	DataLayer_Work(_MSTime, &_Server->m_DataLayer);
	TransportLayer_Work(_MSTime, &_Server->m_TransportLayer);
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
	TransportLayer_Dispose(&_Server->m_TransportLayer);
	NetworkLayer_Dispose(&_Server->m_NetworkLayer);
	DataLayer_Dispose(&_Server->m_DataLayer);

	LinkedList_Node* currentNode = _Server->m_Sockets.m_Head;
	while(currentNode != NULL)
	{
		TCPSocket* TCPSocket = currentNode->m_Item;
		currentNode = currentNode->m_Next;

		TCPSocket_Dispose(TCPSocket);
		LinkedList_RemoveFirst(&_Server->m_Sockets);
	}

	TCPServer_Dispose(&_Server->m_TCPServer);

	LinkedList_Dispose(&_Server->m_Sockets);
	Buffer_Dispose(&_Server->m_Buffer);

	if(_Server->m_Allocated == True)
		Allocator_Free(_Server);
	else
		memset(_Server, 0, sizeof(Filesystem_Server));

}
