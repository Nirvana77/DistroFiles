#include "Filesystem_Server.h"

int Filesystem_Server_InitializePtr(const char* _Path, Filesystem_Server** _ServerPtr)
{
	Filesystem_Server* _Server = (Filesystem_Server*)Allocator_Malloc(sizeof(Filesystem_Server));
	if(_Server == NULL)
		return -1;
	
	int success = Filesystem_Server_Initialize(_Server, _Path);
	if(success != 0)
	{
		Allocator_Free(_Server);
		return success;
	}
	
	_Server->m_Allocated = True;
	
	*(_ServerPtr) = _Server;
	return 0;
}

int Filesystem_Server_Initialize(Filesystem_Server* _Server, const char* _Path)
{
	_Server->m_Allocated = False;

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

	return 0;
}

void Filesystem_Server_Dispose(Filesystem_Server* _Server)
{
	String_Dispose(&_Server->m_FilesytemPath);
	String_Dispose(&_Server->m_Path);

	if(_Server->m_Allocated == True)
		Allocator_Free(_Server);
	else
		memset(_Server, 0, sizeof(Filesystem_Server));

}