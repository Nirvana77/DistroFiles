#include "DistroFiles_Client.h"

int DistroFiles_Client_ConnectedSocket(TCPSocket* _TCPSocket, void* _Context);
int DistroFiles_Client_ConnectionEvent(EventHandler* _EventHandler, int _EventCall, void* _Object, void* _Context);
int DistroFiles_Client_ServerEvent(EventHandler* _EventHandler, int _EventCall, void* _Object, void* _Context);

int DistroFiles_Client_TCPRead(void* _Context, Buffer* _Buffer, int _Size);
int DistroFiles_Client_TCPWrite(void* _Context, Buffer* _Buffer, int _Size);

int DistroFiles_Client_ReveicePayload(void* _Context, Payload* _Message, Payload* _Replay);

int DistroFiles_Client_InitializePtr(DistroFiles_Service* _Service, DistroFiles_Client** _ClientPtr)
{
	DistroFiles_Client* _Client = (DistroFiles_Client*)Allocator_Malloc(sizeof(DistroFiles_Client));
	if(_Client == NULL)
		return -1;
	
	int success = DistroFiles_Client_Initialize(_Client, _Service);
	if(success != 0)
	{
		Allocator_Free(_Client);
		return success;
	}
	
	_Client->m_Allocated = True;
	
	*(_ClientPtr) = _Client;
	return 0;
}

int DistroFiles_Client_Initialize(DistroFiles_Client* _Client, DistroFiles_Service* _Service)
{
	_Client->m_Allocated = False;
	_Client->m_NextCheck = 0;
	_Client->m_Timeout = SEC * 10;
	_Client->m_Service = _Service;
	_Client->m_Server = _Service->m_Server;
	_Client->m_Hook = NULL;

	int success = TCPServer_Initialize(&_Client->m_TCPServer, DistroFiles_Client_ConnectedSocket, _Client);
	if(success != 0)
	{
		printf("Failed to initialize the TCPServer for Server!\n\r");
		printf("Error code: %i\n\r", success);
		return -2;
	}

	success = TCPServer_Listen(&_Client->m_TCPServer, "127.0.0.1", _Service->m_Settings.m_Distributer);
	if(success != 0)
	{
		printf("Failed to listen to port %u for server!\n\r", _Client->m_Service->m_Settings.m_Host);
		printf("Error code: %i\n\r", success);
		TCPServer_Dispose(&_Client->m_TCPServer);
		return -3;
	}

	Bus_Initialize(&_Client->m_Bus);

	success = DataLayer_Initialize(&_Client->m_DataLayer, NULL, Bus_OnRead, Bus_OnWrite, NULL, &_Client->m_Bus, DistroFiles_DatalayerWorkTime);
	if(success != 0)
	{
		printf("Failed to initialize the DataLayer for server!\n\r");
		printf("Error code: %i\n\r", success);
		TCPServer_Disconnect(&_Client->m_TCPServer);
		Bus_Dispose(&_Client->m_Bus);
		return -5;
	}

	success = NetworkLayer_Initialize(&_Client->m_NetworkLayer);
	if(success != 0)
	{
		printf("Failed to initialize the NetworkLayer for server!\n\r");
		printf("Error code: %i\n\r", success);
		TCPServer_Disconnect(&_Client->m_TCPServer);
		Bus_Dispose(&_Client->m_Bus);
		DataLayer_Dispose(&_Client->m_DataLayer);
		return -6;
	}

	success = TransportLayer_Initialize(&_Client->m_TransportLayer);
	if(success != 0)
	{
		printf("Failed to initialize the TransportLayer for server!\n\r");
		printf("Error code: %i\n\r", success);
		TCPServer_Disconnect(&_Client->m_TCPServer);
		DataLayer_Dispose(&_Client->m_DataLayer);
		Bus_Dispose(&_Client->m_Bus);
		NetworkLayer_Dispose(&_Client->m_NetworkLayer);
		return -6;
	}
	
	LinkedList_Initialize(&_Client->m_Connections);
	EventHandler_Initialize(&_Client->m_EventHandler);

	Payload_FuncOut_Set(&_Client->m_DataLayer.m_FuncOut, NetworkLayer_ReveicePayload, NetworkLayer_SendPayload, &_Client->m_NetworkLayer);
	Payload_FuncOut_Set(&_Client->m_NetworkLayer.m_FuncOut, TransportLayer_ReveicePayload, TransportLayer_SendPayload, &_Client->m_TransportLayer);
	Payload_FuncOut_Set(&_Client->m_TransportLayer.m_FuncOut, DistroFiles_Client_ReveicePayload, NULL, _Client);

	_Client->m_Hook = EventHandler_Hook(&_Client->m_Server->m_EventHandler, DistroFiles_Client_ServerEvent, _Client);

	return 0;
}

int DistroFiles_Client_ConnectedSocket(TCPSocket* _TCPSocket, void* _Context)
{
	DistroFiles_Client* _Client = (DistroFiles_Client*) _Context;

	DistroFiles_Connection* _Connection = NULL;
	if(DistroFiles_Connection_InitializePtr(_Client->m_Service->m_Worker, _TCPSocket, &_Client->m_Bus, &_Connection) != 0)
		return 1;

	EventHandler_Hook(&_Connection->m_EventHandler, DistroFiles_Client_ConnectionEvent, _Client);

	LinkedList_Push(&_Client->m_Connections, _Connection);
	return 0;
}

int DistroFiles_Client_ConnectionEvent(EventHandler* _EventHandler, int _EventCall, void* _Object, void* _Context)
{
	DistroFiles_Connection* _Connection = (DistroFiles_Connection*) _Object;
	DistroFiles_Client* _Client = (DistroFiles_Client*) _Context;

	switch (_EventCall)
	{
		case DistroFiles_Connection_Event_Disposed:
		{
			LinkedList_RemoveItem(&_Client->m_Connections, _Connection);
			return 1;
		} break;

		case DistroFiles_Connection_Event_Disconnected:
		{
			printf("Client: ");
			if(_Connection->m_Addrass.m_Type == Payload_Address_Type_IP)
			{
				Connection_Reconnect(_Connection);
				return 0;
			}
			else
			{
				printf("Connection ");
				if(_Connection->m_Addrass.m_Type == Payload_Address_Type_MAC)
				{
					char mac[18];
					Payload_GetMac(&_Connection->m_Addrass, mac);
					printf("\"%s\" ", mac);
				}
				else
				{
					printf("\"\" ");
				}
				printf("got disconnected\r\n");
				LinkedList_RemoveItem(&_Client->m_Connections, _Connection);
				DistroFiles_Connection_Dispose(_Connection);
				return 0;
			}
		}
	}
	return 0;
}


int DistroFiles_Client_ServerEvent(EventHandler* _EventHandler, int _EventCall, void* _Object, void* _Context)
{
	DistroFiles_Client* _Client = (DistroFiles_Client*) _Context;
	DistroFiles_Server_Event _Event = (DistroFiles_Server_Event) _EventCall;
	DistroFiles_Server* _Server = (DistroFiles_Server*) _Object;

	switch (_Event)
	{
		case DistroFiles_Server_Event_Update:
		{
			Payload* message = NULL;
			if(TransportLayer_CreateMessage(&_Client->m_TransportLayer, Payload_Type_ACK, 0, SEC * 10, &message) == 0)
			{
				printf("send update\r\n");
				Payload_SetMessageType(message, Payload_Message_Type_String, "Update", strlen("Update"));
			}
		} break;
	}

	return 0;
}

/*
int DistroFiles_Client_TCPRead(void* _Context, Buffer* _Buffer, int _Size)
{
	DistroFiles_Client* _Client = (DistroFiles_Client*) _Context;
	return DistroFiles_Service_TCPRead(_Client->m_Service, &_Client->m_Connections, _Buffer, _Size);
}

int DistroFiles_Client_TCPWrite(void* _Context, Buffer* _Buffer, int _Size)
{
	DistroFiles_Client* _Client = (DistroFiles_Client*) _Context;
	return DistroFiles_Service_TCPWrite(_Client->m_Service, &_Client->m_Connections, _Buffer, _Size);
}
*/

int DistroFiles_Client_ReveicePayload(void* _Context, Payload* _Message, Payload* _Replay)
{
	DistroFiles_Client* _Client = (DistroFiles_Client*) _Context;

	printf("DistroFiles_Client_ReveicePayload(%i)\n\r", _Message->m_Message.m_Type);
	if(_Message->m_Message.m_Type != Payload_Message_Type_String)
		return 0;

	printf("Method: %s\n\r", _Message->m_Message.m_Method.m_Str);

	if(strcmp(_Message->m_Message.m_Method.m_Str, "upload") == 0)
	{
		Bool isFile = True;
		UInt16 size = 0;

		Buffer_ReadUInt8(&_Message->m_Data, (UInt8*)&isFile);
		Buffer_ReadUInt16(&_Message->m_Data, &size);

		char path[size + 1];
		Buffer_ReadBuffer(&_Message->m_Data, (unsigned char*)path, size);
		path[size] = 0;

		DistroFiles_Server_Write(_Client->m_Server, isFile, path, &_Message->m_Data);
		
	}
	else if(strcmp(_Message->m_Message.m_Method.m_Str, "list") == 0)
	{
		UInt16 size = 0;
		Buffer_ReadUInt16(&_Message->m_Data, &size);

		char path[size + 1];
		Buffer_ReadBuffer(&_Message->m_Data, (unsigned char*)path, size);
		path[size] = 0;

		_Replay->m_Size += DistroFiles_Server_GetList(_Client->m_Server, path, &_Replay->m_Data);
		Payload_SetMessageType(_Replay, Payload_Message_Type_String, "listRespons", strlen("listRespons"));
		return 1;
	}
	else if(strcmp(_Message->m_Message.m_Method.m_Str, "get") == 0)
	{
		Bool isFile = True;
		UInt16 size = 0;

		Buffer_ReadUInt8(&_Message->m_Data, (UInt8*)&isFile);
		Buffer_ReadUInt16(&_Message->m_Data, &size);

		char path[size + 1];
		Buffer_ReadBuffer(&_Message->m_Data, (unsigned char*)path, size);
		path[size] = 0;

		printf("Path: %s\r\n", path);
	}
	else if(strcmp(_Message->m_Message.m_Method.m_Str, "delete") == 0)
	{
		Bool isFile = True;
		Buffer_ReadUInt8(&_Message->m_Data, (UInt8*)&isFile);

		UInt16 size = 0;
		Buffer_ReadUInt16(&_Message->m_Data, &size);

		char path[size + 1];

		Buffer_ReadBuffer(&_Message->m_Data, (unsigned char*)path, size);
		path[size] = 0;

		int success = DistroFiles_Server_Delete(_Client->m_Server, isFile, path);

		_Replay->m_Size += Buffer_WriteUInt8(&_Replay->m_Data, (UInt8)success);
		Payload_SetMessageType(_Replay, Payload_Message_Type_String, "deleteRespons", strlen("deleteRespons"));
	}
	else
	{
		printf("Server can't handel method: %s\n\r", _Message->m_Message.m_Method.m_Str);
	}

	return 0;
}

void DistroFiles_Client_Work(UInt64 _MSTime, DistroFiles_Client* _Client)
{
	TCPServer_Work(&_Client->m_TCPServer);
	DataLayer_Work(_MSTime, &_Client->m_DataLayer);
	TransportLayer_Work(_MSTime, &_Client->m_TransportLayer);

}

void DistroFiles_Client_Dispose(DistroFiles_Client* _Client)
{
	if(_Client->m_Hook != NULL)
	{
		EventHandler_UnHook(&_Client->m_Server->m_EventHandler, _Client->m_Hook);
		_Client->m_Hook = NULL;
	}

	TransportLayer_Dispose(&_Client->m_TransportLayer);
	NetworkLayer_Dispose(&_Client->m_NetworkLayer);
	DataLayer_Dispose(&_Client->m_DataLayer);

	LinkedList_Node* currentNode = _Client->m_Connections.m_Head;
	while(currentNode != NULL)
	{
		DistroFiles_Connection* connection = (DistroFiles_Connection*)currentNode->m_Item;
		currentNode = currentNode->m_Next;

		LinkedList_RemoveFirst(&_Client->m_Connections);
		DistroFiles_Connection_Dispose(connection);
	}
	LinkedList_Dispose(&_Client->m_Connections);
	EventHandler_Dispose(&_Client->m_EventHandler);
	Bus_Dispose(&_Client->m_Bus);

	TCPServer_Dispose(&_Client->m_TCPServer);


	if(_Client->m_Allocated == True)
		Allocator_Free(_Client);
	else
		memset(_Client, 0, sizeof(DistroFiles_Client));

}