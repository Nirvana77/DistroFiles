#include "Filesystem_Server.h"

void Filesystem_Server_Work(UInt64 _MSTime, void* _Context);

int Filesystem_Server_Load(Filesystem_Server* _Server);
int Filesystem_Server_Read(Filesystem_Server* _Server, json_t* _JSON);
int Filesystem_Server_Save(Filesystem_Server* _Server);

int Filesystem_Server_InitializePtr(StateMachine* _Worker, const char* _Path, Filesystem_Server** _ServerPtr)
{
	Filesystem_Server* _Server = (Filesystem_Server*)Allocator_Malloc(sizeof(Filesystem_Server));
	if(_Server == NULL)
		return -1;
	
	int success = Filesystem_Server_Initialize(_Server, _Worker, _Path);
	if(success != 0)
	{
		Allocator_Free(_Server);
		return success;
	}
	
	_Server->m_Allocated = True;
	
	*(_ServerPtr) = _Server;
	return 0;
}

int Filesystem_Server_Initialize(Filesystem_Server* _Server, StateMachine* _Worker, const char* _Path)
{
	_Server->m_Allocated = False;
	_Server->m_Json = NULL;

	int success = String_Initialize(&_Server->m_Path, 32);

	if(success != 0)
	{
		printf("Can't initialize path: %i\n\r", success);
		return -2;
	}

	String_Set(&_Server->m_Path, _Path);

	success = Folder_Create(_Server->m_Path.m_Ptr);

	if(success < 0)
	{
		printf("Can't create folder(%i): %s\n\r", success, _Server->m_Path.m_Ptr);
		String_Dispose(&_Server->m_Path);
		
		return -3;
	}

	String_Initialize(&_Server->m_FilesytemPath, 32);
	
	success = String_Sprintf(&_Server->m_FilesytemPath, "%s/root", _Server->m_Path.m_Ptr);

	success = Folder_Create(_Server->m_FilesytemPath.m_Ptr);

	if(success < 0)
	{
		printf("Can't create folder(%i): %s\n\r", success, _Server->m_FilesytemPath.m_Ptr);
		String_Dispose(&_Server->m_FilesytemPath);
		String_Dispose(&_Server->m_Path);

		return -4;
	}

	int loadSuccess = Filesystem_Server_Load(_Server);
	if(loadSuccess < 0)
	{
		printf("Failed to save Filesystem Server standard settings!\r\n");
		printf("Failed code: %i\n\r", loadSuccess);
		Filesystem_Server_Dispose(_Server);
		return -5;
	}
	else if(loadSuccess == 1)
	{
		Filesystem_Server_Save(_Server);
	}

	StateMachine_CreateTask(_Worker, 0, "FilesystemServer", Filesystem_Server_Work, _Server, &_Server->m_Task);

	return 0;
}

void Filesystem_Server_Work(UInt64 _MSTime, void* _Context)
{
	Filesystem_Server* _Server = (Filesystem_Server*) _Context;

	printf("Work%u\n\r", _MSTime);

	return 0;
}

int Filesystem_Server_Load(Filesystem_Server* _Server)
{
	String str;
	if(String_Initialize(&str, 512) != 0)
		return -1;
		
	char filepath[128];
	sprintf(filepath, "%s/settings.json", _Server->m_Path.m_Ptr);
	
	int success = String_ReadFromFile(&str, filepath);
	if(success != 0)
	{
		Filesystem_Server_Save(_Server);
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
	
	success = Filesystem_Server_Read(_Server, json);
	_Server->m_Json = json;
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

int Filesystem_Server_Read(Filesystem_Server* _Server, json_t* _JSON)
{
	unsigned int version;
	
	if(json_getUInt(_JSON, "version", &version) == 0)
	{
		if(version != Filesystem_Server_VERSION)
			return 2;
	}
	else
	{
		return 1;
	}
	
	Bool needSave = False;
	const char* charVal;
	int intVal;
	
	return needSave == True ? 1 : 0;
}

int Filesystem_Server_Save(Filesystem_Server* _Server)
{
	String str;
	if(String_Initialize(&str, 512) != 0)
		return -1;

	printf("2.1\n\r");
	if(String_Sprintf(&str, 
		"{"
			"\"version\": %u"
		"}",
		Filesystem_Server_VERSION
	) != 0)
	{
		String_Dispose(&str);
		return -1;
	}
	
	printf("2.2\n\r");
	json_error_t error;
	json_t* json = json_loads(str.m_Ptr, 0, &error);
	if(json == NULL)
	{
		printf("Failed to parse new created settings data!");
		printf("JSON: \"%s\"\r\n\r\n", str.m_Ptr);
		String_Dispose(&str);
		return -2;
	}
	
	printf("2.3\n\r");
	char filepath[128];
	sprintf(filepath, "%s/settings.json", _Server->m_Path.m_Ptr);
	int success = String_SaveToFile(&str, filepath);
	
	printf("2.4\n\r");
	if(success != 0)
	{
		printf("Error then saveing new json!\r\n");
		String_Dispose(&str);
		return -3;
	}
	
	json_decref(_Server->m_Json);
	_Server->m_Json = json;
	printf("Save string(%i): %s\r\n", success, str.m_Ptr);
	
	String_Dispose(&str);
	return 0;
}

void Filesystem_Server_Dispose(Filesystem_Server* _Server)
{

	if(_Server->m_Json != NULL)
		json_decref(_Server->m_Json);

	String_Dispose(&_Server->m_FilesytemPath);
	String_Dispose(&_Server->m_Path);

	if(_Server->m_Allocated == True)
		Allocator_Free(_Server);
	else
		memset(_Server, 0, sizeof(Filesystem_Server));

}