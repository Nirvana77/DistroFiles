#include "Filesystem_Service.h"

int Filesystem_Service_Work(UInt64 _MSTime, void* _Context);

int Filesystem_Service_Load(Filesystem_Service* _Service);
int Filesystem_Service_Read(Filesystem_Service* _Service, json_t* _JSON);
int Filesystem_Service_Save(Filesystem_Service* _Service);

int Filesystem_Service_InitializePtr(StateMachine* _Worker, const char* _Path, Filesystem_Service** _ServicePtr)
{
	Filesystem_Service* _Service = (Filesystem_Service*)Allocator_Malloc(sizeof(Filesystem_Service));
	if(_Service == NULL)
		return -1;
	
	int success = Filesystem_Service_Initialize(_Service, _Worker, _Path);
	if(success != 0)
	{
		Allocator_Free(_Service);
		return success;
	}
	
	_Service->m_Allocated = True;
	
	*(_ServicePtr) = _Service;
	return 0;
}

int Filesystem_Service_Initialize(Filesystem_Service* _Service, StateMachine* _Worker, const char* _Path)
{
	_Service->m_Allocated = False;
	_Service->m_Json = NULL;
	_Service->m_Task = NULL;
	_Service->m_Server = NULL;
	_Service->m_Client = NULL;
	_Service->m_Worker = _Worker;

	int success = String_Initialize(&_Service->m_Path, 32);

	if(success != 0)
	{
		printf("Can't initialize path: %i\n\r", success);
		return -2;
	}

	String_Set(&_Service->m_Path, _Path);
	
	success = Folder_Create(_Service->m_Path.m_Ptr);

	if(success < 0)
	{
		printf("Can't create folder(%i): %s\n\r", success, _Service->m_Path.m_Ptr);
		String_Dispose(&_Service->m_Path);
		
		return -3;
	}
	
	char tempFolder[_Service->m_Path.m_Length + 5 + 1];
	sprintf(tempFolder, "%s/temp", _Service->m_Path.m_Ptr);
	success = Folder_Create(tempFolder);
	if(success < 0)
	{
		printf("Can't create folder(%i): %s\n\r", success, tempFolder);
		String_Dispose(&_Service->m_Path);
		
		return -4;
	}
	success = Buffer_Initialize(&_Service->m_Buffer, True, 64);
	if(success != 0)
	{
		printf("Failed to initialize the Buffer!\n\r");
		printf("Error code: %i\n\r", success);
		String_Dispose(&_Service->m_Path);
		return -5;
	}

	//-------------Initialize------------------
	_Service->m_Settings.m_Servers = NULL;

	//-----------------------------------------


	//-----------Default Settings--------------

	_Service->m_Settings.m_Host = 5566;
	_Service->m_Settings.m_Distributer = 8021;
	_Service->m_Settings.m_AutoSync = True;
	_Service->m_Settings.m_Interval = SEC * 10;

	//-----------------------------------------
	int loadSuccess = Filesystem_Service_Load(_Service);
	if(loadSuccess < 0)
	{
		printf("Failed to save Filesystem Server standard settings!\r\n");
		printf("Failed code: %i\n\r", loadSuccess);
		String_Dispose(&_Service->m_Path);
		Buffer_Dispose(&_Service->m_Buffer);
		return -6;
	}
	else if(loadSuccess == 1)
	{
		Filesystem_Service_Save(_Service);
	}

	success = Filesystem_Server_InitializePtr(_Service, &_Service->m_Server);

	if(success != 0)
	{
		printf("Failed to initialize server!\r\n");
		printf("Failed code: %i\n\r", success);
		String_Dispose(&_Service->m_Path);
		Buffer_Dispose(&_Service->m_Buffer);
		return -7;
	}

	success = Filesystem_Client_InitializePtr(_Service, &_Service->m_Client);
	if(success != 0)
	{
		printf("Failed to initialize client!\r\n");
		printf("Failed code: %i\n\r", success);
		String_Dispose(&_Service->m_Path);
		Filesystem_Server_Dispose(_Service->m_Server);
		Buffer_Dispose(&_Service->m_Buffer);
		return -8;
	}

	EventHandler_Initialize(&_Service->m_EventHandler);
	pthread_attr_t attr;
	pthread_attr_init(&attr);


	StateMachine_CreateTask(_Service->m_Worker, &attr, "FilesystemServer", Filesystem_Service_Work, _Service, &_Service->m_Task);
	return 0;
}

int Filesystem_Service_Work(UInt64 _MSTime, void* _Context)
{
	Filesystem_Service* _Service = (Filesystem_Service*) _Context;

	Filesystem_Server_Work(_MSTime, _Service->m_Server);
	Filesystem_Client_Work(_MSTime, _Service->m_Client);
	return 0;
}


int Filesystem_Service_Load(Filesystem_Service* _Service)
{
	String str;
	if(String_Initialize(&str, 512) != 0)
		return -1;
		
	char filepath[128];
	sprintf(filepath, "%s/settings.json", _Service->m_Path.m_Ptr);
	
	int success = String_ReadFromFile(&str, filepath);
	if(success != 0)
	{
		Filesystem_Service_Save(_Service);
		String_Dispose(&str);
		return 0;
	}
	
	json_error_t error;
	json_t* json = json_loads(str.m_Ptr, 0, &error);
	if(json == NULL)
	{
		String_Dispose(&str);
		return -5;
	}
	
	success = Filesystem_Service_Read(_Service, json);
	_Service->m_Json = json;
	String_Dispose(&str);
	
	if(success < 0)
	{
		printf("Failed to read settings.json! errorcode: %i\r\n", success);
		return -6;
	}
	else if(success == 2)
	{
		printf("Wrong version in settings.json!\r\n");
		return -7;
	}
	
	if(success == 1) //Need save!
		return 1;
	
	return 0;
}

int Filesystem_Service_Read(Filesystem_Service* _Service, json_t* _JSON)
{
	UInt8 version;
	
	if(json_getUInt(_JSON, "version", &version) == 0)
	{
		if(version != Filesystem_Service_VERSION)
			return 2;
	}
	else
	{
		return 1;
	}
	
	Bool needSave = False;
	//const char* charVal;
	Bool boolVal = False;
	UInt16 ulintVal;
	if(json_getUInt16(_JSON, "host", &ulintVal) == 0)
	{
		_Service->m_Settings.m_Host = ulintVal;
	}
	else
	{
		needSave = True;
	}

	if(json_getUInt16(_JSON, "distributer", &ulintVal) == 0)
	{
		_Service->m_Settings.m_Distributer = ulintVal;
	}
	else
	{
		needSave = True;
	}

	json_t* servers = json_object_get(_JSON, "servers");
	if(servers != NULL)
	{
		_Service->m_Settings.m_Servers = json_copy(servers);
	}
	else
	{
		needSave = True;
	}

	if(json_getBool(_JSON, "autosync", &boolVal) == 0)
	{
		_Service->m_Settings.m_AutoSync = boolVal;
	}
	else
	{
		needSave = True;
	}

	if(json_getUInt16(_JSON, "interval", &ulintVal) == 0)
	{
		_Service->m_Settings.m_Interval = ulintVal;
	}
	else
	{
		needSave = True;
	}
	
	return needSave == True ? 1 : 0;
}

int Filesystem_Service_Save(Filesystem_Service* _Service)
{
	String str;
	if(String_Initialize(&str, 16) != 0)
		return -1;
	

	if(String_Sprintf(&str, 
		"{"
			"\"version\": %u,"
			"\"host\": %u,"
			"\"distributer\": %u,"
			"\"servers\": [],"
			"\"autosync\": %s,"
			"\"interval\": %u"
		"}",Filesystem_Service_VERSION, _Service->m_Settings.m_Host, _Service->m_Settings.m_Distributer, _Service->m_Settings.m_AutoSync == True ? "true" : "false", _Service->m_Settings.m_Interval
	) != 0)
	{
		String_Dispose(&str);
		return -1;
	}
	
	json_error_t error;
	json_t* json = json_loads(str.m_Ptr, 0, &error);
	if(json == NULL)
	{
		printf("Failed to parse new created settings data!");
		printf("JSON: \"%s\"\r\n\r\n", str.m_Ptr);
		String_Dispose(&str);
		return -2;
	}
	
	char filepath[128];
	char oldfilepath[128];
	sprintf(filepath, "%s/settings.json", _Service->m_Path.m_Ptr);
	sprintf(oldfilepath, "%s/settings.json.old", _Service->m_Path.m_Ptr);
	if(File_Exist(oldfilepath) == True)
	{
		File_Remove(oldfilepath);
		if(File_Copy(filepath, oldfilepath) != 0)
		{
			printf("Error then copying old json!\r\n");
			String_Dispose(&str);
			return -3;
		}
	}
	printf("Save settings.json to settings.json.old\r\n");
	int success = String_SaveToFile(&str, filepath);
	
	if(success != 0)
	{
		printf("Error then saveing new json!\r\n");
		String_Dispose(&str);
		return -4;
	}
	
	json_decref(_Service->m_Json);
	_Service->m_Json = json;
	printf("Save string(%i): %s\r\n", success, str.m_Ptr);
	
	String_Dispose(&str);
	return 0;
}

int Filesystem_Service_TCPRead(Filesystem_Service* _Service, LinkedList* _List, Buffer* _Buffer, int _Size)
{
	if(_List->m_Size == 0)
		return 0;

	int totalReaded = 0;
	LinkedList_Node* currentNode = _List->m_Head;
	unsigned char buffer[Filesystem_Service_BufferMax];
	Buffer_Clear(&_Service->m_Buffer);
	while(currentNode != NULL)
	{
		int readed = 0;
		Filesystem_Connection* connection = (Filesystem_Connection*)currentNode->m_Item;
		readed = TCPSocket_Read(connection->m_Socket, buffer, sizeof(buffer));
		if(readed > 0) {
			totalReaded += Buffer_WriteBuffer(&_Service->m_Buffer, buffer, readed);
			if(connection->m_Addrass.m_Type == Payload_Address_Type_NONE)
			{
				UInt8 flag = 0;
				void* ptr = buffer;
				ptr += Memory_ParseUInt8(ptr, &flag);
				if(BitHelper_GetBit(&flag, 0) == True) {
					ptr += Payload_SourcePosistion;
					ptr += Memory_ParseUInt8(ptr, (UInt8*)&connection->m_Addrass.m_Type);
					ptr += Memory_ParseBuffer(&connection->m_Addrass.m_Address, ptr, sizeof(connection->m_Addrass.m_Address));
				}
			}

		}

		while (readed == Filesystem_Service_BufferMax)
		{
			readed = TCPSocket_Read(connection->m_Socket, buffer, sizeof(buffer));
			totalReaded += Buffer_WriteBuffer(&_Service->m_Buffer, buffer, readed);
		}

		currentNode = currentNode->m_Next;
	}

	if(totalReaded > 0)
	{
		printf("Filesystem_Service_TCPRead\n\r");
		Buffer_DeepCopy(_Buffer, &_Service->m_Buffer, _Service->m_Buffer.m_BytesLeft);
		return totalReaded;
	}

	return 0;
}

int Filesystem_Service_TCPWrite(Filesystem_Service* _Service, LinkedList* _List, Buffer* _Buffer, int _Size)
{
	if(_List->m_Size == 0)
		return 0;

	void* ptr = _Buffer->m_ReadPtr;
	UInt8 flags = 0;
	ptr += Memory_ParseUInt8(ptr, &flags);
	ptr += Payload_DestinationPosistion;

	Payload_Address des;
	UInt8 type = 0;
	ptr += Memory_ParseUInt8(ptr, &type);
	des.m_Type = (Payload_Address_Type)type;

	if(des.m_Type != Payload_Address_Type_NONE)
		ptr += Memory_ParseBuffer(&des.m_Address, ptr, sizeof(des.m_Address));

	LinkedList_Node* currentNode = _List->m_Head;
	Buffer_ResetReadPtr(_Buffer);
	while (currentNode != NULL)
	{
		Filesystem_Connection* connection = (Filesystem_Connection*) currentNode->m_Item;
		currentNode = currentNode->m_Next;

		if(connection->m_Addrass.m_Type == Payload_Address_Type_NONE || des.m_Type == Payload_Address_Type_NONE || CommperIP(connection->m_Addrass.m_Address.MAC, des.m_Address.MAC) == True)
			TCPSocket_Write(connection->m_Socket, _Buffer->m_ReadPtr, _Buffer->m_BytesLeft);
		
		if(connection->m_Socket->m_Status == TCPSocket_Status_Closed)
		{
			printf("Removeing connection(%i) ", connection->m_Socket->m_FD);

			for (int i = 0; i < sizeof(connection->m_Addrass.m_Address); i++)
				printf("%x.", connection->m_Addrass.m_Address.MAC[i]);

			printf("\r\n");
			
			LinkedList_RemoveItem(_List, connection);
			TCPSocket_Dispose(connection->m_Socket);
			Allocator_Free(connection);
			
		}

	}

	return 0;
}

void Filesystem_Service_Dispose(Filesystem_Service* _Service)
{
	if(_Service->m_Task != NULL)
	{
		StateMachine_RemoveTask(_Service->m_Worker, _Service->m_Task);
		_Service->m_Task = NULL;
	}

	if(_Service->m_Server != NULL)
	{
		Filesystem_Server_Dispose(_Service->m_Server);
		_Service->m_Server = NULL;
	}

	if(_Service->m_Client != NULL)
	{
		Filesystem_Client_Dispose(_Service->m_Client);
		_Service->m_Client = NULL;
	}

	Buffer_Dispose(&_Service->m_Buffer);
	EventHandler_Dispose(&_Service->m_EventHandler);

	if(_Service->m_Settings.m_Servers != NULL)
		json_decref(_Service->m_Settings.m_Servers);

	if(_Service->m_Json != NULL)
		json_decref(_Service->m_Json);

	String_Dispose(&_Service->m_Path);

	if(_Service->m_Allocated == True)
		Allocator_Free(_Service);
	else
		memset(_Service, 0, sizeof(Filesystem_Service));

}