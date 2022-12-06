#include "Filesystem_Connection.h"

int Filesystem_Connection_Work(UInt64 _MSTime, void* _Context);
int Filesystem_Connection_OnRead(void* _Context, Buffer* _Buffer);
int Filesystem_Connection_OnWrite(void* _Context, Buffer* _Buffer);

int Filesystem_Connection_InitializePtr(StateMachine* _Worker, TCPSocket* _Socket, Bus* _Bus, Filesystem_Connection** _CommectionPtr)
{
	Filesystem_Connection* _Commection = (Filesystem_Connection*)Allocator_Malloc(sizeof(Filesystem_Connection));
	if(_Commection == NULL)
		return -1;
	
	int success = Filesystem_Connection_Initialize(_Commection, _Worker, _Socket, _Bus);
	if(success != 0)
	{
		Allocator_Free(_Commection);
		return success;
	}
	
	_Commection->m_Allocated = True;
	
	*(_CommectionPtr) = _Commection;
	return 0;
}

int Filesystem_Connection_Initialize(Filesystem_Connection* _Connection, StateMachine* _Worker, TCPSocket* _Socket, Bus* _Bus)
{
	_Connection->m_Allocated = False;
	_Connection->m_Disposed = False;
	_Connection->m_Socket = _Socket;
	_Connection->m_Worker = _Worker;
	_Connection->m_Bus = _Bus;
	
	_Connection->m_Func = NULL;

	memset(&_Connection->m_Addrass, 0, sizeof(Payload_Address));
	
	EventHandler_Initialize(&_Connection->m_EventHandler);
	Buffer_Initialize(&_Connection->m_Buffer, 255); //TODO Make this a define

	int success = Bus_AddFuncIn(_Connection->m_Bus, Filesystem_Connection_OnRead, Filesystem_Connection_OnWrite, _Connection, &_Connection->m_Func);
	if(success != 0)
	{
		printf("Error when createing FuncIn\r\n");
		printf("Error: %i\r\n", success);
		EventHandler_Dispose(&_Connection->m_EventHandler);
		return -2;
	}
	
	success = StateMachine_CreateTask(_Connection->m_Worker, NULL, Filesystem_Connection_Work, _Connection, &_Connection->m_Task);
	if(success != 0)
	{
		printf("Error when createing task\r\n");
		printf("Error: %i\r\n", success);
		EventHandler_Dispose(&_Connection->m_EventHandler);
		return -2;
	}
	
	return 0;
}

int Filesystem_Connection_Work(UInt64 _MSTime, void* _Context)
{
	Filesystem_Connection* _Connection = (Filesystem_Connection*)_Context;

	int readed = TCPSocket_Read((void*)_Connection->m_Socket, &_Connection->m_Buffer);

	if(readed > 0)
	{
		if(_Connection->m_Addrass.m_Type == Payload_Address_Type_NONE)
		{
			void* ptr = _Connection->m_Buffer.m_ReadPtr;
			ptr += Payload_SourcePosistion + 1; //1 because we skip to readed the flags
			UInt8 type = 0;
			ptr += Memory_ParseUInt8(ptr, &type);
			_Connection->m_Addrass.m_Type = (Payload_Address_Type)type;

			if(type != Payload_Address_Type_NONE)
				Memory_ParseBuffer(&_Connection->m_Addrass.m_Address, ptr, sizeof(_Connection->m_Addrass.m_Address));
			
			printf("Added connection(%i) ", _Connection->m_Socket->m_FD);

			for (int i = 0; i < sizeof(_Connection->m_Addrass.m_Address); i++)
				printf("%x.", _Connection->m_Addrass.m_Address.MAC[i]);

			printf("\r\n");
		}
		EventHandler_EventCall(&_Connection->m_EventHandler, Filesystem_Connection_Event_Readed, _Connection);
	}

	if(_MSTime > _Connection->m_NextCheck + Filesystem_Connection_Timeout)
	{
		_Connection->m_NextCheck = _MSTime;
		readed = recv(_Connection->m_Socket->m_FD, NULL, 1, MSG_PEEK | MSG_DONTWAIT);

		if(readed == 0)
		{
			EventHandler_EventCall(&_Connection->m_EventHandler, Filesystem_Connection_Event_Disconnected, _Connection);
		}
	}

	return 0;
}

int Filesystem_Connection_OnRead(void* _Context, Buffer* _Buffer)
{
	Filesystem_Connection* _Connection = (Filesystem_Connection*)_Context;
	if(_Connection->m_Buffer.m_BytesLeft != 0)
	{
		int readed = Buffer_DeepCopy(_Buffer, &_Connection->m_Buffer, 0);
		Buffer_Clear(&_Connection->m_Buffer);
		return readed;
	}

	return 0;
}

int Filesystem_Connection_OnWrite(void* _Context, Buffer* _Buffer)
{
	Filesystem_Connection* _Connection = (Filesystem_Connection*)_Context;
	return TCPSocket_Write((void*)_Connection->m_Socket, _Buffer);	
}

void Filesystem_Connection_Dispose(Filesystem_Connection* _Connection)
{
	if(_Connection->m_Disposed == True)
		return;
	
	_Connection->m_Disposed = True;
	EventHandler_EventCall(&_Connection->m_EventHandler, Filesystem_Connection_Event_Disposed, _Connection);

	Bus_RemoveFuncIn(_Connection->m_Bus, _Connection->m_Func);
	Buffer_Dispose(&_Connection->m_Buffer);

	EventHandler_Dispose(&_Connection->m_EventHandler);
	TCPSocket_Dispose(_Connection->m_Socket);
	StateMachine_RemoveTask(_Connection->m_Worker, _Connection->m_Task);

	if(_Connection->m_Allocated == True)
		Allocator_Free(_Connection);
	else
		memset(_Connection, 0, sizeof(Filesystem_Connection));

}