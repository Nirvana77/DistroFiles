#include "Filesystem_Server.h"

int Filesystem_Server_InitializePtr(const char* _Path, Filesystem_Server** _ServerPtr)
{
	Filesystem_Server* _Server = (struct Filesystem_Server*)malloc(sizeof(Filesystem_Server));
	if(_Server == NULL)
		return -1;
	
	int success = Filesystem_Server_Initialize(_Server, _Path);
	if(success != 0)
	{
		free(_Server);
		return success;
	}
	
	_Server->m_Allocated = True;
	
	*(_ServerPtr) = _Server;
	return 0;
}

int Filesystem_Server_Initialize(Filesystem_Server* _Server, const char* _Path)
{
	_Server->m_Allocated = False;
	return 0;
}

void Filesystem_Server_Dispose(Filesystem_Server* _Server)
{
	

	if(_Server->m_Allocated == True)
		free(_Server);
	else
		memset(_Server, 0, sizeof(Filesystem_Server));

}