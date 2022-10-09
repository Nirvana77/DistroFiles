#include "Filesystem_Client.h"

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
	_Cilent->m_Service = _Service;

	//TODO: Change this do not be hard coded
	TCPClient_Initialize(&_Cilent->m_TCPClient, "127.0.0.1", 5566);

	TCPClient_Connect(&_Cilent->m_TCPClient);

	StateMachine_CreateTask();

	return 0;
}

void Filesystem_Client_Work(UInt64 _MSTime, Filesystem_Client* _Client)
{

}


void Filesystem_Client_Dispose(Filesystem_Client* _Client)
{
	if

	if(_Client->m_Allocated == True)
		Allocator_Free(_Client);
	else
		memset(_Client, 0, sizeof(Filesystem_Client));

}