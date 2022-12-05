#include "Filesystem_Connection.h"

int Filesystem_Connection_Work(UInt64 _MSTime, void* _Context);

int Filesystem_Connection_InitializePtr(StateMachine* _Worker, TCPSocket* _Socket, Buffer* _Buffer, Filesystem_Connection** _CommectionPtr)
{
	Filesystem_Connection* _Commection = (Filesystem_Connection*)Allocator_Malloc(sizeof(Filesystem_Connection));
	if(_Commection == NULL)
		return -1;
	
	int success = Filesystem_Connection_Initialize(_Commection, _Worker, _Socket);
	if(success != 0)
	{
		Allocator_Free(_Commection);
		return success;
	}
	
	_Commection->m_Allocated = True;
	
	*(_CommectionPtr) = _Commection;
	return 0;
}

int Filesystem_Connection_Initialize(Filesystem_Connection* _Connection, StateMachine* _Worker, TCPSocket* _Socket, Buffer* _Buffer)
{
	_Connection->m_Allocated = False;
	_Connection->m_Socket = _Socket;
	_Connection->m_Worker = _Worker;
	_Connection->m_Buffer = _Buffer;

	
	EventHandler_Initialize(&_Connection->m_EventHandler);

	int success = StateMachine_CreateTask(_Connection->m_Worker, NULL, Filesystem_Connection_Work, _Connection, &_Connection->m_Task);
	if(success != 0)
	{
		printf("Error when createing task\r\n");
		printf("Error: %i\r\n", success);
		EventHandler_Dispose(&_Connection->m_EventHandler);
		return -2;
	}
	
	return 0;
}

//! This function needs to be async! 
int Filesystem_Connection_Work(UInt64 _MSTime, void* _Context)
{
	Filesystem_Connection* _Connection = (Filesystem_Connection*)_Context;

	int size = Buffer_SizeLeft(_Connection->m_Buffer);
	int readed = TCPSocket_Read(_Connection->m_Socket, _Connection->m_Buffer->m_WritePtr, size);
	
	while (readed == size)
	{
		Buffer_Extend(_Connection->m_Buffer);
		size = Buffer_SizeLeft(_Connection->m_Buffer);
		readed = TCPSocket_Read(_Connection->m_Socket, _Connection->m_Buffer->m_WritePtr, size);
	}
	
	if(readed > 0)
		EventHandler_EventCall(&_Connection->m_EventHandler, Filesystem_Connection_Event_Readed, _Connection);
		

	if(_MSTime > _Connection->m_NextCheck)
	{
		_Connection->m_NextCheck = _MSTime + Filesystem_Connection_Timeout;

		int succeess = recv(_Connection->m_Socket->m_FD, NULL, 1, MSG_PEEK | MSG_DONTWAIT);

		if(succeess == 0)
		{
			
			EventHandler_EventCall(&_Connection->m_EventHandler, Filesystem_Connection_Event_Disconnected, _Connection);
			return 1;
		}
	}

	return 0;
}

void Filesystem_Connection_Dispose(Filesystem_Connection* _Connection)
{
	EventHandler_Dispose(&_Connection->m_EventHandler);
	StateMachine_RemoveTask(_Connection->m_Worker, _Connection->m_Task);

	if(_Connection->m_Allocated == True)
		Allocator_Free(_Connection);
	else
		memset(_Connection, 0, sizeof(Filesystem_Connection));

}