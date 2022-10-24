#include "Filesystem_Service.h"

void Filesystem_Service_Work(UInt64 _MSTime, void* _Context);

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

	String_Initialize(&_Service->m_FilesytemPath, 32);
	
	success = String_Sprintf(&_Service->m_FilesytemPath, "%s/root", _Service->m_Path.m_Ptr);

	success = Folder_Create(_Service->m_FilesytemPath.m_Ptr);

	if(success < 0)
	{
		printf("Can't create folder(%i): %s\n\r", success, _Service->m_FilesytemPath.m_Ptr);
		String_Dispose(&_Service->m_FilesytemPath);
		String_Dispose(&_Service->m_Path);

		return -4;
	}

	//-------------Initialize------------------
	String_Initialize(&_Service->m_Settings.m_Host.m_IP, 16);
	String_Initialize(&_Service->m_Settings.m_Distributer.m_IP, 16);
	_Service->m_Settings.m_Servers = NULL;

	//-----------------------------------------


	//-----------Default Settings--------------

	_Service->m_Settings.m_Host.m_Port = 5566;
	_Service->m_Settings.m_Distributer.m_Port = 5566;

	String_Set(&_Service->m_Settings.m_Host.m_IP, "127.0.0.1");
	String_Set(&_Service->m_Settings.m_Distributer.m_IP, "127.0.0.1");

	//-----------------------------------------
	int loadSuccess = Filesystem_Service_Load(_Service);
	if(loadSuccess < 0)
	{
		printf("Failed to save Filesystem Server standard settings!\r\n");
		printf("Failed code: %i\n\r", loadSuccess);
		String_Dispose(&_Service->m_FilesytemPath);
		String_Dispose(&_Service->m_Path);
		String_Dispose(&_Service->m_Settings.m_Distributer.m_IP);
		String_Dispose(&_Service->m_Settings.m_Host.m_IP);
		return -5;
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
		String_Dispose(&_Service->m_FilesytemPath);
		String_Dispose(&_Service->m_Path);
		String_Dispose(&_Service->m_Settings.m_Distributer.m_IP);
		String_Dispose(&_Service->m_Settings.m_Host.m_IP);
		return -6;
	}

	success = Filesystem_Client_InitializePtr(_Service, &_Service->m_Client);

	if(success != 0)
	{
		printf("Failed to initialize client!\r\n");
		printf("Failed code: %i\n\r", success);
		String_Dispose(&_Service->m_FilesytemPath);
		String_Dispose(&_Service->m_Path);
		String_Dispose(&_Service->m_Settings.m_Distributer.m_IP);
		String_Dispose(&_Service->m_Settings.m_Host.m_IP);
		Filesystem_Server_Dispose(_Service->m_Server);
		return -6;
	}

	StateMachine_CreateTask(_Service->m_Worker, 0, "FilesystemServer", Filesystem_Service_Work, _Service, &_Service->m_Task);
	return 0;
}

void Filesystem_Service_Work(UInt64 _MSTime, void* _Context)
{
	Filesystem_Service* _Service = (Filesystem_Service*) _Context;

	Filesystem_Server_Work(_MSTime, _Service->m_Server);
	Filesystem_Client_Work(_MSTime, _Service->m_Client);
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
	const char* charVal;
	//UInt8 uintVal;
	UInt16 ulintVal;

	json_t* host = json_object_get(_JSON, "host");
	if(host != NULL)
	{
		if(json_getString(host, "IP", &charVal) == 0)
		{
			String_Set(&_Service->m_Settings.m_Host.m_IP, charVal);
		}
		else
		{
			needSave = True;
		}

		if(json_getUInt16(host, "port", &ulintVal) == 0)
		{
			_Service->m_Settings.m_Host.m_Port = ulintVal;
		}
		else
		{
			needSave = True;
		}
	}
	else
	{
		needSave = True;
	}

	json_t* distributer = json_object_get(_JSON, "distributer");
	if(distributer != NULL)
	{
		if(json_getString(distributer, "IP", &charVal) == 0)
		{
			String_Set(&_Service->m_Settings.m_Distributer.m_IP, charVal);
		}
		else
		{
			needSave = True;
		}

		if(json_getUInt16(distributer, "port", &ulintVal) == 0)
		{
			_Service->m_Settings.m_Distributer.m_Port = ulintVal;
		}
		else
		{
			needSave = True;
		}
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
	
	return needSave == True ? 1 : 0;
}

int Filesystem_Service_Save(Filesystem_Service* _Service)
{
	String str;
	if(String_Initialize(&str, 16) != 0)
		return -1;

	String servers;
	
	if(String_Initialize(&servers, 16) != 0)
		return -1;
	
	char tempStr[126];
	char ip[17];
	LinkedList_Node* currentNode = _Service->m_Server->m_Sockets.m_Head;
	while (currentNode != NULL)
	{
		TCPSocket* socket = (TCPSocket*) currentNode->m_Item;
		memset(ip, 0, sizeof(ip));
		inet_ntop(AF_INET, &socket->m_Addr.sin_addr.s_addr, ip, sizeof(ip));
		
		sprintf(tempStr, "{\"post\": %i,\"IP\": %s}", ntohs(socket->m_Addr.sin_port), ip);
		String_Append(&servers, tempStr, strlen(tempStr));

		currentNode = currentNode->m_Next;
		if(currentNode != NULL)
			String_Append(&servers, ",", 1);
	}
	

	if(String_Sprintf(&str, 
		"{"
			"\"version\": %u,"
			"\"host\": {"
				"\"port\": %u,"
				"\"IP\": \"%s\""
			"},"
			"\"distributer\": {"
				"\"port\": %u,"
				"\"IP\": \"%s\""
			"},"
			"\"servers\": ["
				"%s"
			"]"
		"}",Filesystem_Service_VERSION, _Service->m_Settings.m_Host.m_Port, _Service->m_Settings.m_Host.m_IP.m_Ptr, _Service->m_Settings.m_Distributer.m_Port, _Service->m_Settings.m_Distributer.m_IP.m_Ptr, servers.m_Ptr
	) != 0)
	{
		String_Dispose(&str);
		String_Dispose(&servers);
		return -1;
	}
	String_Dispose(&servers);
	
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
	sprintf(filepath, "%s/settings.json", _Service->m_Path.m_Ptr);
	int success = String_SaveToFile(&str, filepath);
	
	if(success != 0)
	{
		printf("Error then saveing new json!\r\n");
		String_Dispose(&str);
		return -3;
	}
	
	json_decref(_Service->m_Json);
	_Service->m_Json = json;
	printf("Save string(%i): %s\r\n", success, str.m_Ptr);
	
	String_Dispose(&str);
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

	String_Dispose(&_Service->m_Settings.m_Distributer.m_IP);
	String_Dispose(&_Service->m_Settings.m_Host.m_IP);

	if(_Service->m_Settings.m_Servers != NULL)
		json_decref(_Service->m_Settings.m_Servers);

	if(_Service->m_Json != NULL)
		json_decref(_Service->m_Json);

	String_Dispose(&_Service->m_FilesytemPath);
	String_Dispose(&_Service->m_Path);

	if(_Service->m_Allocated == True)
		Allocator_Free(_Service);
	else
		memset(_Service, 0, sizeof(Filesystem_Service));

}