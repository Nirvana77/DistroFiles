#include "Filesystem_Server.h"

int Filesystem_Server_ConnectedSocket(TCPSocket* _TCPSocket, void* _Context);

int Filesystem_Server_TCPRead(void* _Context, Buffer* _Buffer, int _Size);
int Filesystem_Server_TCPWrite(void* _Context, Buffer* _Buffer, int _Size);
int Filesystem_Server_LoadServer(Filesystem_Server* _Server);

int Filesystem_Server_ReveicePayload(void* _Context, Payload* _Message, Payload* _Replay);

int Filesystem_Server_GetConnection(Filesystem_Server* _Server, Payload_Address* _Address, Filesystem_Connection** _ConnectionPtr);

int Filesystem_Server_ReadFile(Filesystem_Server* _Server, String* _FullPath, Buffer* _DataBuffer,  Payload* _Replay);
int Filesystem_Server_ReadFolder(Filesystem_Server* _Server, String* _FullPath, Buffer* _DataBuffer,  Payload* _Replay);
int Filesystem_Server_WriteFile(Filesystem_Server* _Server, String* _FullPath, Buffer* _DataBuffer);
int Filesystem_Server_WriteFolder(Filesystem_Server* _Server, String* _FullPath, Buffer* _DataBuffer);


int Filesystem_Server_ForwordWrite(Filesystem_Server* _Server, Payload_Address* _IgnoreAddress, Bool _IsFile, unsigned char* _Path, unsigned char _Hash[16]);
int Filesystem_Server_ForwordDelete(Filesystem_Server* _Server, Payload_Address* _IgnoreAddress, Bool _IsFile, char* _Path, unsigned char _Hash[16]);
void Filesystem_Server_Forwording(Filesystem_Server* _Server, Payload_Address* _IgnoreAddress, Buffer* _Data);

int Filesystem_Server_MessageEvent(EventHandler* _EventHandler, int _EventCall, void* _Object, void* _Context);


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
	_Server->m_TempFlag = 0;
	_Server->m_TempListSize = 0;
	_Server->m_NextCheck = 0;
	_Server->m_Timeout = SEC * 10;
	_Server->m_Service = _Service;
	_Server->m_State = Filesystem_Server_State_Init;

	BitHelper_SetBit(&_Server->m_TempFlag, Filesystem_Server_TempFlag_HasList, False);
	BitHelper_SetBit(&_Server->m_TempFlag, Filesystem_Server_TempFlag_WorkonList, False);
	BitHelper_SetBit(&_Server->m_TempFlag, Filesystem_Server_TempFlag_WillSend, False);
	BitHelper_SetBit(&_Server->m_TempFlag, Filesystem_Server_TempFlag_WillClear, False);

	int success = TCPServer_Initialize(&_Server->m_TCPServer, Filesystem_Server_ConnectedSocket, _Server);
	if(success != 0)
	{
		printf("Failed to initialize the TCPServer for Server!\n\r");
		printf("Error code: %i\n\r", success);
		return -2;
	}

	success = TCPServer_Listen(&_Server->m_TCPServer, "127.0.0.1", _Service->m_Settings.m_Host);
	if(success != 0)
	{
		printf("Failed to listen to port %u for server!\n\r", _Server->m_Service->m_Settings.m_Host);
		printf("Error code: %i\n\r", success);
		TCPServer_Dispose(&_Server->m_TCPServer);
		return -3;
	}
	

	String_Initialize(&_Server->m_FilesytemPath, 32);
	
	success = String_Sprintf(&_Server->m_FilesytemPath, "%s/root", _Service->m_Path.m_Ptr);

	success = Folder_Create(_Server->m_FilesytemPath.m_Ptr);

	if(success < 0)
	{
		printf("Can't create folder(%i): %s\n\r", success, _Server->m_FilesytemPath.m_Ptr);
		String_Dispose(&_Server->m_FilesytemPath);

		return -4;
	}

	success = Buffer_Initialize(&_Server->m_TempListBuffer, True, 64);
	if(success != 0)
	{
		printf("Failed to initialize the Buffer!\n\r");
		printf("Error code: %i\n\r", success);
		TCPServer_Disconnect(&_Server->m_TCPServer);
		String_Dispose(&_Server->m_FilesytemPath);
		return -4;
	}

	success = Filesystem_Checking_Initialize(&_Server->m_Checking, _Server);
	if(success != 0)
	{
		printf("Failed to initialize the DataLayer for server!\n\r");
		printf("Error code: %i\n\r", success);
		TCPServer_Disconnect(&_Server->m_TCPServer);
		String_Dispose(&_Server->m_FilesytemPath);
		return -5;
	}

	success = DataLayer_Initialize(&_Server->m_DataLayer, NULL, Filesystem_Server_TCPRead, Filesystem_Server_TCPWrite, NULL, _Server, MS * 100);
	if(success != 0)
	{
		printf("Failed to initialize the DataLayer for server!\n\r");
		printf("Error code: %i\n\r", success);
		TCPServer_Disconnect(&_Server->m_TCPServer);
		String_Dispose(&_Server->m_FilesytemPath);
		Filesystem_Checking_Dispose(&_Server->m_Checking);
		Buffer_Dispose(&_Server->m_TempListBuffer);
		return -5;
	}

	success = NetworkLayer_Initialize(&_Server->m_NetworkLayer);
	if(success != 0)
	{
		printf("Failed to initialize the NetworkLayer for server!\n\r");
		printf("Error code: %i\n\r", success);
		TCPServer_Disconnect(&_Server->m_TCPServer);
		String_Dispose(&_Server->m_FilesytemPath);
		Filesystem_Checking_Dispose(&_Server->m_Checking);
		Buffer_Dispose(&_Server->m_TempListBuffer);
		DataLayer_Dispose(&_Server->m_DataLayer);
		return -6;
	}

	success = TransportLayer_Initialize(&_Server->m_TransportLayer);
	if(success != 0)
	{
		printf("Failed to initialize the TransportLayer for server!\n\r");
		printf("Error code: %i\n\r", success);
		TCPServer_Disconnect(&_Server->m_TCPServer);
		String_Dispose(&_Server->m_FilesytemPath);
		Filesystem_Checking_Dispose(&_Server->m_Checking);
		Buffer_Dispose(&_Server->m_TempListBuffer);
		DataLayer_Dispose(&_Server->m_DataLayer);
		NetworkLayer_Dispose(&_Server->m_NetworkLayer);
		return -6;
	}

	
	LinkedList_Initialize(&_Server->m_Connections);
	EventHandler_Initialize(&_Server->m_EventHandler);

	Payload_FuncOut_Set(&_Server->m_DataLayer.m_FuncOut, NetworkLayer_ReveicePayload, NetworkLayer_SendPayload, &_Server->m_NetworkLayer);
	Payload_FuncOut_Set(&_Server->m_NetworkLayer.m_FuncOut, TransportLayer_ReveicePayload, TransportLayer_SendPayload, &_Server->m_TransportLayer);
	Payload_FuncOut_Set(&_Server->m_TransportLayer.m_FuncOut, Filesystem_Server_ReveicePayload, NULL, _Server);

	char tempPath[_Server->m_Service->m_Path.m_Length + 5];
	sprintf(tempPath, "%s/temp", _Server->m_Service->m_Path.m_Ptr);

	if(Folder_IsEmpty(tempPath) == False)
		BitHelper_SetBit(&_Server->m_TempFlag, Filesystem_Server_TempFlag_HasList, True);

	return 0;
}

int Filesystem_Server_ConnectedSocket(TCPSocket* _TCPSocket, void* _Context)
{
	Filesystem_Server* _Server = (Filesystem_Server*) _Context;

	Filesystem_Connection* connection = (Filesystem_Connection*)Allocator_Malloc(sizeof(Filesystem_Connection));

	connection->m_Socket = _TCPSocket;
	memset(&connection->m_Addrass, 0, sizeof(Payload_Address));

	LinkedList_Push(&_Server->m_Connections, connection);
	return 0;
}

int Filesystem_Server_TCPRead(void* _Context, Buffer* _Buffer, int _Size)
{
	Filesystem_Server* _Server = (Filesystem_Server*) _Context;
	return Filesystem_Service_TCPRead(_Server->m_Service, &_Server->m_Connections, _Buffer, _Size);
}

//note: This is a bite janked
int Filesystem_Server_TCPWrite(void* _Context, Buffer* _Buffer, int _Size)
{
	Filesystem_Server* _Server = (Filesystem_Server*) _Context;
	LinkedList list;
	LinkedList_Initialize(&list);
	if(_Server->m_State == Filesystem_Server_State_ReSyncing)
	{
		LinkedList_Node* currentNode = _Server->m_Connections.m_Head;
		while (currentNode != NULL)
		{
			Filesystem_Connection* connection = (Filesystem_Connection*)currentNode->m_Item;

			Bool isAllowed = Filesystem_Checking_CanUseConnection(&_Server->m_Checking, connection);

			if(isAllowed == True)
			{
				LinkedList_Node* node = currentNode;
				currentNode = currentNode->m_Next;
				LinkedList_UnlinkNode(&_Server->m_Connections, node);
				LinkedList_LinkFirst(&list, node);
			}
			else
			{
				currentNode = currentNode->m_Next;
			}
			
		}
		
	}
	else
	{
		LinkedList_Node* currentNode = _Server->m_Connections.m_Head;
		while (currentNode != NULL)
		{
			currentNode = currentNode->m_Next;
			LinkedList_LinkFirst(&list, LinkedList_UnlinkFirst(&_Server->m_Connections));
		}
	}

	int success = Filesystem_Service_TCPWrite(_Server->m_Service, &list, _Buffer, _Size);

	LinkedList_Node* currentNode = list.m_Head;
	while (currentNode != NULL)
	{
		currentNode = currentNode->m_Next;
		LinkedList_LinkFirst(&_Server->m_Connections, LinkedList_UnlinkFirst(&list));
	}
	

	LinkedList_Dispose(&list);
	return success;
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
			Filesystem_Connection* connection = (Filesystem_Connection*)Allocator_Malloc(sizeof(Filesystem_Connection));

			connection->m_Socket = socket;
			memset(&connection->m_Addrass, 0, sizeof(Payload_Address));
			LinkedList_Push(&_Server->m_Connections, connection);
		}
		
	}

	Payload* message = NULL;
	TransportLayer_CreateMessage(&_Server->m_TransportLayer, Payload_Type_Broadcast, 0, SEC, &message);
	
	_Server->m_State = Filesystem_Server_State_Conneced;
	return 0;
}

//TODO: Fix sync quantity check
int Filesystem_Server_ReveicePayload(void* _Context, Payload* _Message, Payload* _Replay)
{
	Filesystem_Server* _Server = (Filesystem_Server*) _Context;

	if(_Message->m_Message.m_Type != Payload_Message_Type_String)
	{
		printf("Filesystem_Server_ReveicePayload(%i)\n\r", _Message->m_Message.m_Type);
		return 0;
	}

	printf("Method: %s\n\r", _Message->m_Message.m_Method.m_Str);

	if(strcmp(_Message->m_Message.m_Method.m_Str, "SyncAck") == 0)
	{
		UInt8 isOk = 0;

		Buffer_ReadUInt8(&_Message->m_Data, &isOk);

		if(isOk == 0) {
			SystemMonotonicMS(&_Server->m_LastSynced);
			_Server->m_State = Filesystem_Server_State_Synced;
			return 0;
		}

		UInt16 size = 0;
		Buffer_ReadUInt16(&_Message->m_Data, &size);

		char path[size + 1];
		Buffer_ReadBuffer(&_Message->m_Data, (unsigned char*)path, size);
		path[size] = 0;

		if(BitHelper_GetBit(&_Server->m_TempFlag, Filesystem_Server_TempFlag_HasList) == True &&
		   BitHelper_GetBit(&_Server->m_TempFlag, Filesystem_Server_TempFlag_WorkonList) == True)
		{
			void* ptr = _Server->m_TempListBuffer.m_Ptr;
			UInt16 pathSize = 0;
			ptr += Memory_ParseUInt16(ptr, &pathSize);
			
			char workingPath[pathSize + 1];
			ptr += Memory_ParseBuffer(workingPath, ptr, pathSize);
			workingPath[pathSize - 5] = 0;

			if(strcmp(&workingPath[17], path) == 0)
				return 0;

		}

		String filePath;
		String_Initialize(&filePath, 64);
		String_Append(&filePath, path, strlen(path));
		String_Append(&filePath, ".data", strlen(".data"));
		String_Exchange(&filePath, "/", "_");
		char filename[filePath.m_Length + 1];
		strcpy(filename, filePath.m_Ptr);
		filename[filePath.m_Length] = 0;

		String_Set(&filePath, _Server->m_Service->m_Path.m_Ptr);

		if(String_EndsWith(&filePath, "/") == False)
			String_Append(&filePath, "/", 1);
			
		String_Append(&filePath, "temp/List_", strlen("temp/List_"));
		String_Append(&filePath, filename, strlen(filename));

		FILE* f = NULL;
		File_Open(filePath.m_Ptr, File_Mode_ReadWriteCreateBinary, &f);
		
		printf("SyncAck path: %s\n\r", filePath.m_Ptr);
		if(f != NULL)
		{
			Buffer listData;
			Buffer_Initialize(&listData, False, _Message->m_Data.m_BytesLeft + 8);
			Buffer_ReadUInt16(&_Message->m_Data, &size);

			Buffer_WriteUInt16(&listData, size);
			
			int written = Buffer_WriteBuffer(&listData, _Message->m_Data.m_ReadPtr, _Message->m_Data.m_BytesLeft);

			_Message->m_Data.m_ReadPtr += written;
			_Message->m_Data.m_BytesLeft -= written;

			File_WriteAll(f, listData.m_ReadPtr, listData.m_BytesLeft);

			File_Close(f);
			Buffer_Dispose(&listData);


			if(BitHelper_GetBit(&_Server->m_TempFlag, Filesystem_Server_TempFlag_HasList) == False)
			{
				BitHelper_SetBit(&_Server->m_TempFlag, Filesystem_Server_TempFlag_HasList, True);
			}
			else
			{
				_Server->m_TempListSize--;

				unsigned char hash[16];
				Buffer_ReadBuffer(&_Server->m_TempListBuffer, hash, 16);

				BitHelper_SetBit(&_Server->m_TempFlag, Filesystem_Server_TempFlag_WillSend, True);

			}
		}
		String_Dispose(&filePath);
	}
	else if(strcmp(_Message->m_Message.m_Method.m_Str, "Sync") == 0)
	{
		String fullPath;
		String_Initialize(&fullPath, 64);

		String_Set(&fullPath, _Server->m_FilesytemPath.m_Ptr);

		UInt16 size = 0;
		Buffer_ReadUInt16(&_Message->m_Data, &size);

		char path[size + 1];
		Buffer_ReadBuffer(&_Message->m_Data, (unsigned char*)path, size);
		path[size] = 0;

		if(strcmp(path, "root") != 0)
		{
			if(String_EndsWith(&fullPath, "/") == False)
				String_Append(&fullPath, "/", 1);

			String_Append(&fullPath, path, strlen(path));
		}
		
		unsigned char hash[16];
		Buffer_ReadBuffer(&_Message->m_Data, hash, 16);
		
		unsigned char serverHash[16];
		Folder_Hash(fullPath.m_Ptr, serverHash);

		Payload_SetMessageType(_Replay, Payload_Message_Type_String, "SyncAck", strlen("SyncAck"));
		_Replay->m_Type = Payload_Type_Respons;

		if(Filesystem_Server_HashCheck(hash, serverHash) == True)
		{
			_Replay->m_Size += Buffer_WriteUInt8(&_Replay->m_Data, 0);
			String_Dispose(&fullPath);
			return 1;
		}
		_Replay->m_Size += Buffer_WriteUInt8(&_Replay->m_Data, 1);
		
		_Replay->m_Size += Buffer_WriteUInt16(&_Replay->m_Data, size);
		_Replay->m_Size += Buffer_WriteBuffer(&_Replay->m_Data, (unsigned char*)path, size);
		
		tinydir_dir dir;
		if(tinydir_open(&dir, fullPath.m_Ptr) != 0) {
			String_Dispose(&fullPath);
			return -1;
		}
		String_Dispose(&fullPath);

		size = 0;
		Buffer folderContext;
		Buffer_Initialize(&folderContext, True, 64);
		while (dir.has_next)
		{
			tinydir_file file;
			tinydir_readfile(&dir, &file);
			if(strcmp(file.name, ".") != 0 && strcmp(file.name, "..") != 0)
			{
				Buffer_WriteUInt8(&folderContext, file.is_dir ? False : True);
				Buffer_WriteUInt16(&folderContext, strlen(file.name));
				Buffer_WriteBuffer(&folderContext, (unsigned char*)file.name, strlen(file.name));
				size++;
			}
			tinydir_next(&dir);
		}

		tinydir_close(&dir);

		_Replay->m_Size += Buffer_WriteUInt16(&_Replay->m_Data, size);
		_Replay->m_Size += Buffer_WriteBuffer(&_Replay->m_Data, folderContext.m_ReadPtr, folderContext.m_BytesLeft);

		Payload_Print(_Replay, "SyncAck list data");

		Buffer_Dispose(&folderContext);
		return 1;
	}
	else if(strcmp(_Message->m_Message.m_Method.m_Str, "Move") == 0 ||
			strcmp(_Message->m_Message.m_Method.m_Str, "Rename") == 0)
	{
		printf("Move/Reanme\n\r");
	}
	else if(strcmp(_Message->m_Message.m_Method.m_Str, "Write") == 0)
	{
		Filesystem_Checking_SetState(&_Server->m_Checking, Filesystem_Checking_Type_Write, _Message);
		Bool isFile = True;
		Buffer_ReadUInt8(&_Message->m_Data, (UInt8*)&isFile);

		UInt16 size = 0;
		Buffer_ReadUInt16(&_Message->m_Data, &size);

		unsigned char fileHash[16] = "";
		unsigned char bufferHash[16] = "";
		unsigned char path[size + 1];
		String fullPath;

		String_Initialize(&fullPath, 64);
		String_Set(&fullPath, _Server->m_FilesytemPath.m_Ptr);

		if(String_EndsWith(&fullPath, "/") == False)
			String_Append(&fullPath, "/", 1);

		Buffer_ReadBuffer(&_Message->m_Data, path, size);
		path[size] = 0;

		String_Append(&fullPath, (const char*)path, size);

		Buffer_ReadUInt16(&_Message->m_Data, &size);
		if(isFile == True)
		{
			if(File_Exist(fullPath.m_Ptr) == True)
			{
				
				File_GetHash(fullPath.m_Ptr, fileHash);
				Memory_ParseBuffer(bufferHash, _Message->m_Data.m_ReadPtr + size, 16);

				if(Filesystem_Server_HashCheck(bufferHash, fileHash) == False)
				{
					File_Remove(fullPath.m_Ptr);
				}
				else
				{
					printf("Write check OK\r\n");
					Filesystem_Server_ForwordWrite(_Server, &_Message->m_Src, isFile, path, fileHash);
					String_Dispose(&fullPath);
					return 0;
				}
			}
		}
		else
		{
			printf("Fix folder write\r\n");
		}
		

		FILE* f = NULL;
		printf("Writing: %s\n\r", fullPath.m_Ptr);
		File_Open((const char*)fullPath.m_Ptr, File_Mode_ReadWriteCreateBinary, &f);

		if(f == NULL)
		{
			printf("Error with write\n\r");
			printf("Can't write to path: %s\n\r", fullPath.m_Ptr);
			String_Dispose(&fullPath);
			return 0;
		}
		
		int written = File_WriteAll(f, _Message->m_Data.m_ReadPtr, size);
		_Message->m_Data.m_ReadPtr += written;
		_Message->m_Data.m_BytesLeft -= written;
		File_Close(f);

		unsigned char hash[16] = "";
		File_GetHash(fullPath.m_Ptr, hash);
		String_Dispose(&fullPath);
		
		Buffer_ReadBuffer(&_Message->m_Data, bufferHash, 16);
		
		if(Filesystem_Server_HashCheck(hash, bufferHash) == False)
			Filesystem_Server_Sync(_Server);
		else
			Filesystem_Server_ForwordWrite(_Server, &_Message->m_Src, isFile, path, hash);

	}
	else if(strcmp(_Message->m_Message.m_Method.m_Str, "Delete") == 0)
	{
		Filesystem_Checking_SetState(&_Server->m_Checking, Filesystem_Checking_Type_Delete, _Message);
		Bool isFile = True;
		Buffer_ReadUInt8(&_Message->m_Data, (UInt8*)&isFile);

		UInt16 size = 0;
		Buffer_ReadUInt16(&_Message->m_Data, &size);

		unsigned char hash[16] = "";
		unsigned char bufferHash[16] = "";
		char path[size + 1];
		String fullPath;

		String_Initialize(&fullPath, 64);
		String_Set(&fullPath, _Server->m_FilesytemPath.m_Ptr);

		if(String_EndsWith(&fullPath, "/") == False)
			String_Append(&fullPath, "/", 1);

		Buffer_ReadBuffer(&_Message->m_Data, (unsigned char*)path, size);
		path[size] = 0;

		String_Append(&fullPath, path, size);

		if(isFile == True)
			File_Remove(fullPath.m_Ptr);
		
		else
			Folder_Remove(fullPath.m_Ptr);
		
		int index = String_LastIndexOf(&fullPath, "/");
		String_SubString(&fullPath, index, fullPath.m_Length);

		Folder_Hash(fullPath.m_Ptr, hash);
		String_Dispose(&fullPath);
		Buffer_ReadBuffer(&_Message->m_Data, bufferHash, 16);
		
		if(Filesystem_Server_HashCheck(hash, bufferHash) == False)
			Filesystem_Server_Sync(_Server);
		else
			Filesystem_Server_ForwordDelete(_Server, &_Message->m_Src, isFile, path, hash);
		
	}
	else if(strcmp(_Message->m_Message.m_Method.m_Str, "Check") == 0)
	{
		UInt8 type = 0;
		Buffer_ReadUInt8(&_Message->m_Data, &type);
		int success = Filesystem_Checking_WorkOnPayload(&_Server->m_Checking, (Filesystem_Checking_Type)type, _Message);
		if(success > 0)
		{
			printf("Success: %i\r\n", success);
			return 2; //* Postponed message for 2 sec
		}
		return success;
	}
	else if(strcmp(_Message->m_Message.m_Method.m_Str, "CheckAck") == 0)
	{
		UInt8 type = 0;
		Buffer_ReadUInt8(&_Message->m_Data, &type);
		
		if(_Server->m_Checking.m_Type == Filesystem_Checking_Type_None)
			return 0; //* Ignore the message

		if((Filesystem_Checking_Type)type != _Server->m_Checking.m_Type)
		{
			printf("Message type: %i\r\nChecking type: %i\r\n", (int)type, (int)_Server->m_Checking.m_Type);
			return 2; //* Postponed message for 2 sec
		}
		
		Filesystem_Checking_Check* check = NULL;
		if(Filesystem_Checking_SpawnWriteCheck(&_Server->m_Checking, &_Message->m_Src, &check) == 1)
			return 0;
		
		Buffer_ReadUInt8(&_Message->m_Data, &type);
		check->m_IsOk = (Filesystem_Checking_Check_Satus)type;
		
		if(Filesystem_Server_GetConnection(_Server, &_Message->m_Src, &check->m_Connection) != 0)
		{
			printf("Error with: \r\n");
			for (int i = 0; i < sizeof(_Message->m_Src.m_Address); i++)
				printf("%x ", _Message->m_Src.m_Address.MAC[i]);
			printf("\r\n");
			return 0;
		}
		
	}
	else if(strcmp(_Message->m_Message.m_Method.m_Str, "ReSync") == 0)
	{
		Filesystem_Server_Sync(_Server);
	}
	else if(strcmp(_Message->m_Message.m_Method.m_Str, "Read") == 0)
	{
		printf("Read\n\r");

		Bool isFile = True;
		Buffer_ReadUInt8(&_Message->m_Data, (UInt8*)&isFile);

		UInt16 size = 0;
		Buffer_ReadUInt16(&_Message->m_Data, &size);

		unsigned char path[size + 1];
		
		String fullPath;

		String_Initialize(&fullPath, 64);
		String_Set(&fullPath, _Server->m_FilesytemPath.m_Ptr);

		if(String_EndsWith(&fullPath, "/") == False)
			String_Append(&fullPath, "/", 1);

		Buffer_ReadBuffer(&_Message->m_Data, path, size);
		path[size] = 0;

		String_Append(&fullPath, (const char*)path, size);

		_Replay->m_Size += Buffer_WriteUInt8(&_Replay->m_Data, (UInt8)isFile);
		_Replay->m_Size += Buffer_WriteUInt16(&_Replay->m_Data, size);
		_Replay->m_Size += Buffer_WriteBuffer(&_Replay->m_Data, path, size);

		int success = -1;
		if(isFile == True)
		{
			success = Filesystem_Server_ReadFile(_Server, &fullPath, &_Message->m_Data, _Replay);
		}
		else
		{
			success = Filesystem_Server_ReadFolder(_Server, &fullPath, &_Message->m_Data, _Replay);

		}
		
		if(success < 0)
			printf("Success error: %i\n\r", success);

		String_Dispose(&fullPath);
		return success;
	}
	else if(strcmp(_Message->m_Message.m_Method.m_Str, "ReadRespons") == 0)
	{
		printf("ReadRespons\n\r");

		Bool isFile = True;
		Buffer_ReadUInt8(&_Message->m_Data, (UInt8*)&isFile);

		UInt16 size = 0;
		Buffer_ReadUInt16(&_Message->m_Data, &size);

		String fullPath;

		String_Initialize(&fullPath, 64);
		String_Set(&fullPath, _Server->m_FilesytemPath.m_Ptr);

		if(String_EndsWith(&fullPath, "/") == False)
			String_Append(&fullPath, "/", 1);

		unsigned char path[size + 1];
		Buffer_ReadBuffer(&_Message->m_Data, path, size);
		path[size] = 0;

		String_Append(&fullPath, (const char*)path, size);

		int success = -1;
		if(isFile == True)
			success = Filesystem_Server_WriteFile(_Server, &fullPath, &_Message->m_Data);
		else
			success = Filesystem_Server_WriteFolder(_Server, &fullPath, &_Message->m_Data);

		if(success < 0)
		{
			printf("Success error: %i\n\r", success);
		}
		else if(success == 0)
		{
			if(BitHelper_GetBit(&_Server->m_TempFlag, Filesystem_Server_TempFlag_WorkonList) == True)
			{
				_Server->m_TempListSize--;
				BitHelper_SetBit(&_Server->m_TempFlag, Filesystem_Server_TempFlag_WillSend, True);

			}
		}

		String_Dispose(&fullPath);
		return success;
	}
	else
	{
		printf("Server can't handel method: %s\n\r", _Message->m_Message.m_Method.m_Str);
	}

	return 0;
}

int Filesystem_Server_GetConnection(Filesystem_Server* _Server, Payload_Address* _Address, Filesystem_Connection** _ConnectionPtr)
{
	LinkedList_Node* currentNode = _Server->m_Connections.m_Head;
	while (currentNode != NULL)
	{
		Filesystem_Connection* connection = (Filesystem_Connection*) currentNode->m_Item;
		currentNode = currentNode->m_Next;

		if(Payload_ComperAddresses(&connection->m_Addrass, _Address) == True)
		{
			if(_ConnectionPtr != NULL)
				*(_ConnectionPtr) = connection;
			return 0;
		}

	}
	
	return 1;
}

int Filesystem_Server_ReadFile(Filesystem_Server* _Server, String* _FullPath, Buffer* _DataBuffer,  Payload* _Replay)
{
	FILE* f = NULL;

	File_Open((const char*)_FullPath->m_Ptr, File_Mode_ReadBinary, &f);

	if(f == NULL)
	{
		printf("Error with read\n\r");
		printf("Can't read to path: %s\n\r", _FullPath->m_Ptr);
		return -1;
	}
	
	_Replay->m_Size += Buffer_WriteUInt16(&_Replay->m_Data, (UInt16)File_GetSize(f));
	_Replay->m_Size += Buffer_ReadFromFile(&_Replay->m_Data, f);
	File_Close(f);

	unsigned char hash[16] = "";
	File_GetHash(_FullPath->m_Ptr, hash);
	_Replay->m_Size += Buffer_WriteBuffer(&_Replay->m_Data, hash, 16);

	Payload_SetMessageType(_Replay, Payload_Message_Type_String, "ReadRespons", strlen("ReadRespons"));
	_Replay->m_Type = Payload_Type_Respons;

	return 1;
}

int Filesystem_Server_ReadFolder(Filesystem_Server* _Server, String* _FullPath, Buffer* _DataBuffer,  Payload* _Replay)
{

	Payload_SetMessageType(_Replay, Payload_Message_Type_String, "ReadRespons", strlen("ReadRespons"));
	_Replay->m_Type = Payload_Type_Respons;
	
	tinydir_dir dir;
	if(tinydir_open(&dir, _FullPath->m_Ptr) != 0)
		return -1;

	UInt16 size = 0;
	Buffer folderContext;
	Buffer_Initialize(&folderContext, True, 64);
	while (dir.has_next)
	{
		tinydir_file file;
		tinydir_readfile(&dir, &file);
		if(strcmp(file.name, ".") != 0 && strcmp(file.name, "..") != 0)
		{
			Buffer_WriteUInt8(&folderContext, file.is_dir ? False : True);
			Buffer_WriteUInt16(&folderContext, strlen(file.name));
			Buffer_WriteBuffer(&folderContext, (unsigned char*)file.name, strlen(file.name));

			unsigned char hash[16];
			if(file.is_dir)
				Folder_Hash(file.path, hash);
			else
				File_GetHash(file.path, hash);

			Buffer_WriteBuffer(&folderContext, hash, 16);
			size++;
		}
		tinydir_next(&dir);
	}

	tinydir_close(&dir);

	_Replay->m_Size += Buffer_WriteUInt16(&_Replay->m_Data, size);
	_Replay->m_Size += Buffer_WriteBuffer(&_Replay->m_Data, folderContext.m_ReadPtr, folderContext.m_BytesLeft);

	Buffer_Dispose(&folderContext);

	return 1;
}

int Filesystem_Server_WriteFile(Filesystem_Server* _Server, String* _FullPath, Buffer* _DataBuffer)
{
	FILE* f = NULL;
	UInt16 size = 0;
	Buffer_ReadUInt16(_DataBuffer, &size);

	if(File_Exist(_FullPath->m_Ptr) == True) {
		unsigned char fileHash[16] = "";
		unsigned char bufferHash[16] = "";
		
		File_GetHash(_FullPath->m_Ptr, fileHash);
		Memory_ParseBuffer(bufferHash, _DataBuffer->m_ReadPtr + size, 16);

		if(Filesystem_Server_HashCheck(bufferHash, fileHash) == False)
			File_Remove(_FullPath->m_Ptr);
		else
			return 0;
	}

	File_Open(_FullPath->m_Ptr, File_Mode_ReadWriteCreateBinary, &f);

	if(f == NULL)
	{
		printf("Error with write\n\r");
		printf("Can't write to path: %s\n\r", _FullPath->m_Ptr);
		return -1;
	}
	int written = File_WriteAll(f, _DataBuffer->m_ReadPtr, size);
	_DataBuffer->m_ReadPtr += written;
	_DataBuffer->m_BytesLeft -= written;
	File_Close(f);
	
	unsigned char hash[16] = "";
	Buffer_ReadBuffer(_DataBuffer, hash, 16);

	unsigned char serverHash[16] = "";
	File_GetHash(_FullPath->m_Ptr, serverHash);

	if(Filesystem_Server_HashCheck(hash, serverHash) == False)
	{
		printf("Hash:\n\r");
		for (int i = 0; i < 16; i++)
			printf("%x ",hash[i]);
		printf("serverHash:\n\r");
		for (int i = 0; i < 16; i++)
			printf("%x ",serverHash[i]);
		
		
		printf("\n\rHash check failed!\n\r");
		File_Remove(_FullPath->m_Ptr);
		return -1;
	}



	return 0;
}

int Filesystem_Server_WriteFolder(Filesystem_Server* _Server, String* _FullPath, Buffer* _DataBuffer)
{

	String str;
	String_Initialize(&str, 32);

	Folder_Create(_FullPath->m_Ptr);

	String_Set(&str, _FullPath->m_Ptr);
	String_Exchange(&str, _Server->m_FilesytemPath.m_Ptr, "");
	String_Exchange(&str, "/", "_");

	char path[str.m_Length];
	sprintf(path, "%s", str.m_Ptr);

	String_Set(&str, _Server->m_Service->m_Path.m_Ptr);

	if(String_EndsWith(&str, "/") == False)
		String_Append(&str, "/", 1);
	
	String_Append(&str, "temp/list", strlen("temp/list"));

	if(strcmp(path, "root") != 0)
		String_Append(&str, "_root", strlen("_root"));

	String_Append(&str, path, strlen(path));
	String_Append(&str, ".data", strlen(".data"));

	FILE* f = NULL;
	File_Open(str.m_Ptr, File_Mode_ReadWriteCreateBinary, &f);
	if(f == NULL)
	{
		printf("Fullpath error(Filesystem_Server_WriteFolder): %s\n\r", str.m_Ptr);
		String_Dispose(&str);
	}


	int written = File_WriteAll(f, _DataBuffer->m_ReadPtr, _DataBuffer->m_BytesLeft);

	_DataBuffer->m_ReadPtr += written;
	_DataBuffer->m_BytesLeft -= written;

	File_Close(f);

	String_Dispose(&str);
	return 0;
}

int Filesystem_Server_GetList(Filesystem_Server* _Server, char* _Path, Buffer* _DataBuffer)
{
	String fullPath;
	String_Initialize(&fullPath, 64);

	String_Set(&fullPath, _Server->m_FilesytemPath.m_Ptr);

	if(String_EndsWith(&fullPath, "/") == False)
		String_Append(&fullPath, "/", 1);
	
	if(strcmp(_Path, "root") != 0)
		String_Append(&fullPath, _Path, strlen(_Path));
		
	if(String_EndsWith(&fullPath, "/") == False)
		String_Append(&fullPath, "/", 1);

	int written = 0;

	
	tinydir_dir dir;
	if(tinydir_open(&dir, fullPath.m_Ptr) != 0)
		return -1;
	String_Dispose(&fullPath);

	UInt16 size = 0;
	Buffer folderContext;
	Buffer_Initialize(&folderContext, True, 64);
	while (dir.has_next)
	{
		tinydir_file file;
		tinydir_readfile(&dir, &file);
		if(strcmp(file.name, ".") != 0 && strcmp(file.name, "..") != 0)
		{
			Buffer_WriteUInt8(&folderContext, file.is_dir ? False : True);
			Buffer_WriteUInt16(&folderContext, strlen(file.name));
			Buffer_WriteBuffer(&folderContext, (unsigned char*)file.name, strlen(file.name));
			size++;
		}
		tinydir_next(&dir);
	}

	tinydir_close(&dir);

	written += Buffer_WriteUInt16(_DataBuffer, size);
	written += Buffer_WriteBuffer(_DataBuffer, folderContext.m_ReadPtr, folderContext.m_BytesLeft);

	Buffer_Dispose(&folderContext);
	return written;
}

//! This gets called from client ONLY
int Filesystem_Server_Write(Filesystem_Server* _Server, Bool _IsFile, char* _Path, Buffer* _DataBuffer)
{
	String fullPath;
	String_Initialize(&fullPath, 64);

	String_Set(&fullPath, _Server->m_FilesytemPath.m_Ptr);

	if(String_EndsWith(&fullPath, "/") == False)
		String_Append(&fullPath, "/", 1);
	
	String_Append(&fullPath, _Path, strlen(_Path));

	int success = 0;
	if(_IsFile == True)
	{
		void* ptr = _DataBuffer->m_ReadPtr;
		success = Filesystem_Server_WriteFile(_Server, &fullPath, _DataBuffer);
		if(success == 0)
		{
			
			UInt16 fileSize = 0;
			ptr += Memory_ParseUInt16(ptr, &fileSize);
			UInt16 size = 1 + 2 + strlen(_Path) + 2 + fileSize + 16;

			Payload* message = NULL;
			if(TransportLayer_CreateMessage(&_Server->m_TransportLayer, Payload_Type_Broadcast, size, SEC, &message) == 0)
			{
				Buffer_WriteUInt8(&message->m_Data, (UInt8)_IsFile);
				
				Buffer_WriteUInt16(&message->m_Data, (UInt16)strlen(_Path));
				Buffer_WriteBuffer(&message->m_Data, (unsigned char*)_Path, (UInt16)strlen(_Path));

				Buffer_WriteUInt16(&message->m_Data, fileSize);
				Buffer_WriteBuffer(&message->m_Data, ptr, fileSize + 16);

				Payload_SetMessageType(message, Payload_Message_Type_String, "Write", strlen("Write"));
			}

		}
	}
	else
	{
		printf("Fix Filesystem_Server_Write for Folders\r\n");
		success = 1;
	}
	
	String_Dispose(&fullPath);
	return success;
}

int Filesystem_Server_ForwordWrite(Filesystem_Server* _Server, Payload_Address* _IgnoreAddress, Bool _IsFile, unsigned char* _Path, unsigned char _Hash[16])
{
	Buffer data;
	Buffer_Initialize(&data, False, 1 + 1 + 2 + strlen((const char*)_Path) + 16);

	Buffer_WriteUInt8(&data, (UInt8) Filesystem_Checking_Type_Write);
	Buffer_WriteUInt8(&data, (UInt8) _IsFile);
	Buffer_WriteUInt16(&data, strlen((const char*)_Path));
	Buffer_WriteBuffer(&data, _Path, strlen((const char*)_Path));
	Buffer_WriteBuffer(&data, _Hash, 16);

	Filesystem_Server_Forwording(_Server, _IgnoreAddress, &data);
	
	Buffer_Dispose(&data);
	return 0;
}

//! This gets called from client ONLY
int Filesystem_Server_Delete(Filesystem_Server* _Server, Bool _IsFile, char* _Path)
{
	String fullPath;
	String_Initialize(&fullPath, 64);

	String_Set(&fullPath, _Server->m_FilesytemPath.m_Ptr);

	if(String_EndsWith(&fullPath, "/") == False)
		String_Append(&fullPath, "/", 1);
	
	String_Append(&fullPath, _Path, strlen(_Path));

	int success = -1;
	if(_IsFile == True)
		success = File_Remove(fullPath.m_Ptr);
	else
		success = Folder_Remove(fullPath.m_Ptr);

	if(success >= 0)
	{
		
		int index = String_LastIndexOf(&fullPath, "/");
		String_SubString(&fullPath, index, fullPath.m_Length);

		unsigned char hash[16] = "";
		Folder_Hash(fullPath.m_Ptr, hash);
		UInt16 size = 1 + 2 + strlen(_Path) + 16;

		Payload* message = NULL;
		if(TransportLayer_CreateMessage(&_Server->m_TransportLayer, Payload_Type_Broadcast, size, SEC, &message) == 0)
		{
			Buffer_WriteUInt8(&message->m_Data, (UInt8)_IsFile);
			
			Buffer_WriteUInt16(&message->m_Data, (UInt16)strlen(_Path));
			Buffer_WriteBuffer(&message->m_Data, (unsigned char*)_Path, (UInt16)strlen(_Path));

			Buffer_WriteBuffer(&message->m_Data, hash, 16);

			Payload_SetMessageType(message, Payload_Message_Type_String, "Delete", strlen("Delete"));
		}
	}

	String_Dispose(&fullPath);
	return 0;
}

int Filesystem_Server_ForwordDelete(Filesystem_Server* _Server, Payload_Address* _IgnoreAddress, Bool _IsFile, char* _Path, unsigned char _Hash[16])
{
	Buffer data;
	Buffer_Initialize(&data, False, 1 + 1 + 2 + strlen((const char*)_Path) + 16);

	Buffer_WriteUInt8(&data, (UInt8) Filesystem_Checking_Type_Delete);
	Buffer_WriteUInt8(&data, (UInt8)_IsFile);
	Buffer_WriteUInt16(&data, strlen((const char*)_Path));
	Buffer_WriteBuffer(&data, (unsigned char*)_Path, strlen((const char*)_Path));
	Buffer_WriteBuffer(&data, _Hash, 16);
	
	Filesystem_Server_Forwording(_Server, _IgnoreAddress, &data);

	Buffer_Dispose(&data);
	return 0;
}

void Filesystem_Server_Forwording(Filesystem_Server* _Server, Payload_Address* _IgnoreAddress, Buffer* _Data)
{
	_Server->m_State = Filesystem_Server_State_Checking;

	LinkedList_Node* currentNode = _Server->m_Connections.m_Head;
	while (currentNode != NULL)
	{
		Filesystem_Connection* connection = (Filesystem_Connection*)currentNode->m_Item;
		currentNode = currentNode->m_Next;

		if (Payload_ComperAddresses(&connection->m_Addrass, _IgnoreAddress) == False)
		{
			Payload* msg = NULL;
			if(TransportLayer_CreateMessage(&_Server->m_TransportLayer, Payload_Type_Safe, _Data->m_BytesLeft, SEC, &msg) == 0)
			{
				
				Buffer_Copy(&msg->m_Data, _Data, _Data->m_BytesLeft);

				Payload_FilAddress(&msg->m_Des, &connection->m_Addrass);
				Payload_SetMessageType(msg, Payload_Message_Type_String, "Check", strlen("Check"));

			}
		}
		

	}
}

void Filesystem_Server_Work(UInt64 _MSTime, Filesystem_Server* _Server)
{
	TCPServer_Work(&_Server->m_TCPServer);
	DataLayer_Work(_MSTime, &_Server->m_DataLayer);
	TransportLayer_Work(_MSTime, &_Server->m_TransportLayer);

	switch (_Server->m_State)
	{
		case Filesystem_Server_State_Init:
		{
			_Server->m_State = Filesystem_Server_State_Connecting;
			Filesystem_Server_LoadServer(_Server);
		} break;

		case Filesystem_Server_State_Conneced:
		{
			_Server->m_State = Filesystem_Server_State_ReSync;
		} break;

		case Filesystem_Server_State_ReSyncing:
		case Filesystem_Server_State_Syncing:
		{

			if(BitHelper_GetBit(&_Server->m_TempFlag, Filesystem_Server_TempFlag_HasList) == True &&
			BitHelper_GetBit(&_Server->m_TempFlag, Filesystem_Server_TempFlag_WorkonList) == False)
			{
				Buffer_Clear(&_Server->m_TempListBuffer);

				
				String fullPath;

				String_Initialize(&fullPath, 64);
				String_Set(&fullPath, _Server->m_Service->m_Path.m_Ptr);

				if(String_EndsWith(&fullPath, "/") == False)
					String_Append(&fullPath, "/", 1);

				String_Append(&fullPath, "temp", strlen("temp"));

				tinydir_dir dir;
				if(tinydir_open(&dir, fullPath.m_Ptr) != 0)
				{
					printf("Fullpath error(HasList): %s\n\r", fullPath.m_Ptr);
					String_Dispose(&fullPath);
					return;
				}

				String_Dispose(&fullPath);
				
				while (dir.has_next)
				{
					tinydir_file file;
					tinydir_readfile(&dir, &file);
					if(strcmp(file.name, ".") != 0 && strcmp(file.name, "..") != 0)
					{
						FILE* f = NULL;
						File_Open(file.path, File_Mode_ReadBinary, &f);
						
						int written = Buffer_WriteUInt16(&_Server->m_TempListBuffer, strlen(file.path));
						written += Buffer_WriteBuffer(&_Server->m_TempListBuffer, (unsigned char*)file.path, strlen(file.path));

						_Server->m_TempListBuffer.m_ReadPtr += written;
						_Server->m_TempListBuffer.m_BytesLeft -= written;

						Buffer_ReadFromFile(&_Server->m_TempListBuffer, f);
						BitHelper_SetBit(&_Server->m_TempFlag, Filesystem_Server_TempFlag_WorkonList, True);

						File_Close(f);
						break;
					}
					tinydir_next(&dir);
				}

				tinydir_close(&dir);

				Buffer_ReadUInt16(&_Server->m_TempListBuffer, &_Server->m_TempListSize);
				BitHelper_SetBit(&_Server->m_TempFlag, Filesystem_Server_TempFlag_WillSend, True);
			}
			else if (BitHelper_GetBit(&_Server->m_TempFlag, Filesystem_Server_TempFlag_WillSend) == True)
			{
				BitHelper_SetBit(&_Server->m_TempFlag, Filesystem_Server_TempFlag_WillSend, False);

				if(_Server->m_TempListSize == 0)
				{
					BitHelper_SetBit(&_Server->m_TempFlag, Filesystem_Server_TempFlag_WillClear, True);
					return;
				}

				Bool isFile = True;
				Buffer_ReadUInt8(&_Server->m_TempListBuffer, (UInt8*)&isFile);

				UInt16 size = 0;
				Buffer_ReadUInt16(&_Server->m_TempListBuffer, &size);
				
				char path[size + 1];
				Buffer_ReadBuffer(&_Server->m_TempListBuffer, (unsigned char*)path, size);
				path[size] = 0;

				String str;
				String_Initialize(&str, 32);
				
				UInt16 length = 0;
				void* ptr = _Server->m_TempListBuffer.m_Ptr;
				ptr += Memory_ParseUInt16(_Server->m_TempListBuffer.m_Ptr, &length);
				char fullPath[length + 1];
				ptr += Memory_ParseBuffer(fullPath, ptr, length);
				fullPath[length] = 0;

				int index = _Server->m_Service->m_Path.m_Length + 6 + 5;

				if(strncmp(&fullPath[index], "root_", 5) == 0)
				{
					String_Set(&str, &fullPath[5 + index]);

					str.m_Ptr[str.m_Length - 5] = 0;
					str.m_Length -= 5;

					String_Append(&str, "/", 1);
					String_Append(&str, path, strlen(path));
				}
				else
				{
					String_Set(&str, path);
				}
				
				Payload* message = NULL;
				if(TransportLayer_CreateMessage(&_Server->m_TransportLayer, Payload_Type_Broadcast, 1 + 2 + str.m_Length, SEC, &message) == 0)
				{

					Buffer_WriteUInt8(&message->m_Data, (UInt8)isFile);
					Buffer_WriteUInt16(&message->m_Data, str.m_Length);
					Buffer_WriteBuffer(&message->m_Data, (unsigned char*)str.m_Ptr, str.m_Length);

					Payload_SetMessageType(message, Payload_Message_Type_String, "Read", strlen("Read"));
				}
				String_Dispose(&str);
			}
			else if(BitHelper_GetBit(&_Server->m_TempFlag, Filesystem_Server_TempFlag_WillClear) == True)
			{
				BitHelper_SetBit(&_Server->m_TempFlag, Filesystem_Server_TempFlag_HasList, False);
				BitHelper_SetBit(&_Server->m_TempFlag, Filesystem_Server_TempFlag_WorkonList, False);
				BitHelper_SetBit(&_Server->m_TempFlag, Filesystem_Server_TempFlag_WillClear, False);
				
				UInt16 length = 0;

				void* ptr = _Server->m_TempListBuffer.m_Ptr;
				ptr += Memory_ParseUInt16(_Server->m_TempListBuffer.m_Ptr, &length);
				char fullPath[length + 1];
				ptr += Memory_ParseBuffer(fullPath, ptr, length);
				fullPath[length] = 0;

				if(File_Remove(fullPath) != 0)
				{
					printf("File error(WillClear): %s\n\r", fullPath);
					return;
				}

				
				String tempPath;

				String_Initialize(&tempPath, 64);
				String_Set(&tempPath, _Server->m_Service->m_Path.m_Ptr);

				if(String_EndsWith(&tempPath, "/") == False)
					String_Append(&tempPath, "/", 1);

				String_Append(&tempPath, "temp", strlen("temp"));

				tinydir_dir dir;
				if(tinydir_open(&dir, tempPath.m_Ptr) != 0)
				{
					printf("TempPath error(HasList): %s\n\r", tempPath.m_Ptr);
					String_Dispose(&tempPath);
					return;
				}

				String_Dispose(&tempPath);
				
				while (dir.has_next)
				{
					tinydir_file file;
					tinydir_readfile(&dir, &file);
					if(strcmp(file.name, ".") != 0 && strcmp(file.name, "..") != 0)
					{
						BitHelper_SetBit(&_Server->m_TempFlag, Filesystem_Server_TempFlag_WorkonList, False);
						BitHelper_SetBit(&_Server->m_TempFlag, Filesystem_Server_TempFlag_HasList, True);

						break;
					}
					tinydir_next(&dir);
				}

				tinydir_close(&dir);

				if(BitHelper_GetBit(&_Server->m_TempFlag, Filesystem_Server_TempFlag_HasList) == False)
					_Server->m_State = Filesystem_Server_State_Synced;
				
				
			}
	

		} break;

		case Filesystem_Server_State_Synced:
		{
			SystemMonotonicMS(&_Server->m_LastSynced);
			_Server->m_State = Filesystem_Server_State_Idel;
			
		} break;

		case Filesystem_Server_State_Checking:
		{
			Filesystem_Checking_Work(_MSTime, &_Server->m_Checking);
		} break;

		case Filesystem_Server_State_ReSync:
		{
			Filesystem_Server_Sync(_Server);
			_Server->m_State = Filesystem_Server_State_ReSyncing;
		} break;

		case Filesystem_Server_State_Idel:
		{
			//printf("Ideling\r\n");
			if(_Server->m_Service->m_Settings.m_AutoSync == True)
			{
				if(_MSTime > _Server->m_LastSynced + _Server->m_Service->m_Settings.m_Interval)
					Filesystem_Server_Sync(_Server);
				
			}
			
		} break;

		case Filesystem_Server_State_Connecting:
		{ } break;

	}
	
	if(_MSTime > _Server->m_NextCheck)
	{
		_Server->m_NextCheck = _MSTime + _Server->m_Timeout;
		LinkedList_Node* currentNode = _Server->m_Connections.m_Head;
		while (currentNode != NULL)
		{
			TCPSocket* socket = (TCPSocket*)currentNode->m_Item;
			currentNode = currentNode->m_Next;

			int succeess = recv(socket->m_FD,NULL,1, MSG_PEEK | MSG_DONTWAIT);

			if(succeess == 0)
			{
				LinkedList_RemoveItem(&_Server->m_Connections, socket);
				TCPSocket_Dispose(socket);
			}
		}
		
	}

}

void Filesystem_Server_Sync(Filesystem_Server* _Server)
{
	Payload* message = NULL;
	char* path = "root";

	int size = 2 + strlen(path) + 16;

	if(TransportLayer_CreateMessage(&_Server->m_TransportLayer, Payload_Type_Broadcast, size, Filesystem_Server_SyncTimeout, &message) == 0)
	{
		_Server->m_State = Filesystem_Server_State_Syncing;
		Buffer_WriteUInt16(&message->m_Data, strlen(path));
		Buffer_WriteBuffer(&message->m_Data, (unsigned char*)path, strlen(path));

		unsigned char hash[16];
		Folder_Hash(_Server->m_FilesytemPath.m_Ptr, hash);

		Filesystem_Server_PrintHash("Sync Hash", hash);
		Buffer_WriteBuffer(&message->m_Data, hash, 16);

		Payload_SetMessageType(message, Payload_Message_Type_String, "Sync", strlen("Sync"));
		EventHandler_Hook(&message->m_EventHandler, Filesystem_Server_MessageEvent, _Server);
	}
}

//note: Then return 1 the event gets unhooked;
int Filesystem_Server_MessageEvent(EventHandler* _EventHandler, int _EventCall, void* _Object, void* _Context)
{
	Filesystem_Server* _Server = (Filesystem_Server*) _Context;
	Payload* _Message = (Payload*) _Object;
	Payload_State _Event = (Payload_Type)_EventCall;
	int success = 0;
	
	switch (_Event)
	{/*
		case Payload_State_Resived:
		{
			
		} break;
		{
			
		} break;
		case Payload_State_Replay:
		{
			
		} break;
		
		case Payload_State_Init:
		case Payload_State_Sented:
		case Payload_State_Sending:
		{
			return 0;
		} break;*/

		case Payload_State_Timeout:
		{
			if(_Message->m_Message.m_Type == Payload_Message_Type_String)
			{
				if(strcmp(_Message->m_Message.m_Method.m_Str, "Sync") == 0)
				{
					Payload* message = NULL;
					if(TransportLayer_ResendMessage(&_Server->m_TransportLayer, _Message, &message) == 0)
					{
						message->m_Timeout = Payload_TimeoutAlgorithm(message->m_Timeout);
					}
				}
			}
			return 1;
		} break;

		case Payload_State_Removed:
		case Payload_State_Destroyed:
		case Payload_State_Failed:
		{
			success = 1;
		} break;

		default: 
		{
			char str[UUID_FULLSTRING_SIZE];
			uuid_ToString(_Message->m_UUID, str);
			printf("Event: %i UUID: %s Server status: %i\r\n", _EventCall, str, _Server->m_State);
		} break;
	}

	return success;
}

void Filesystem_Server_Dispose(Filesystem_Server* _Server)
{
	TransportLayer_Dispose(&_Server->m_TransportLayer);
	NetworkLayer_Dispose(&_Server->m_NetworkLayer);
	DataLayer_Dispose(&_Server->m_DataLayer);

	LinkedList_Node* currentNode = _Server->m_Connections.m_Head;
	while(currentNode != NULL)
	{
		Filesystem_Connection* connection = (Filesystem_Connection*)currentNode->m_Item;
		currentNode = currentNode->m_Next;

		TCPSocket_Dispose(connection->m_Socket);
		Allocator_Free(connection);
		LinkedList_RemoveFirst(&_Server->m_Connections);
	}

	LinkedList_Dispose(&_Server->m_Connections);
	EventHandler_Dispose(&_Server->m_EventHandler);

	TCPServer_Dispose(&_Server->m_TCPServer);

	String_Dispose(&_Server->m_FilesytemPath);

	Filesystem_Checking_Dispose(&_Server->m_Checking);
	Buffer_Dispose(&_Server->m_TempListBuffer);

	if(_Server->m_Allocated == True)
		Allocator_Free(_Server);
	else
		memset(_Server, 0, sizeof(Filesystem_Server));

}
