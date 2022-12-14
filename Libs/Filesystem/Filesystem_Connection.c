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

	_Connection->m_Socket = _Socket;
	_Connection->m_Worker = _Worker;
	_Connection->m_Bus = _Bus;
	
	_Connection->m_Func = NULL;
	_Connection->m_Disposed = False;
	_Connection->m_HasReaded = False;

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
	if(_Connection->m_Disposed == True)
		return 1;

	TCPSocket_Error error;
	int readed = TCPSocket_Read((void*)_Connection->m_Socket, _Connection->m_DataBuffer, TCP_BufferSize, &error);

	if(error.m_HasError == True)
	{
		switch (error.m_Error)
		{
			case ENOTSOCK: //* Not a socket
			case 32: //* Broken pipe
			{
				EventHandler_EventCall(&_Connection->m_EventHandler, Filesystem_Connection_Event_Disconnected, _Connection);
				return 0;
			} break;

			case 0:
			case EWOULDBLOCK:
			{ } break;

			default:
			{
				printf("Other Read error: %i, read is: %i\r\n", error.m_Error, readed);
			} break;
		}
	}
	else if(readed > 0)
	{
		if(_Connection->m_Addrass.m_Type == Payload_Address_Type_NONE)
		{
			void* ptr = _Connection->m_DataBuffer;
			ptr += Payload_SourcePosistion + 1; //1 because we skip to readed the flags
			UInt8 type = 0;
			ptr += Memory_ParseUInt8(ptr, &type);
			if(!(type < Payload_Address_Type_Min || type > Payload_Address_Type_Max))
			{
				_Connection->m_Addrass.m_Type = (Payload_Address_Type)type;
				Memory_ParseBuffer(&_Connection->m_Addrass.m_Address, ptr, sizeof(_Connection->m_Addrass.m_Address));
			}
		}

		EventHandler_EventCall(&_Connection->m_EventHandler, Filesystem_Connection_Event_Readed, _Connection);
		Buffer_WriteBuffer(&_Connection->m_Buffer, _Connection->m_DataBuffer, readed);
		
		if(readed != TCP_BufferSize)
			_Connection->m_HasReaded = True;

	}

	if(_MSTime > _Connection->m_NextCheck + Filesystem_Connection_Timeout)
	{
		_Connection->m_NextCheck = _MSTime;
		readed = recv(_Connection->m_Socket->m_FD, NULL, 1, MSG_PEEK | MSG_DONTWAIT);

		if(readed == 0)
			EventHandler_EventCall(&_Connection->m_EventHandler, Filesystem_Connection_Event_Disconnected, _Connection);
		
	}

	return 0;
}

int Filesystem_Connection_OnRead(void* _Context, Buffer* _Buffer)
{
	Filesystem_Connection* _Connection = (Filesystem_Connection*)_Context;
	if(_Connection->m_HasReaded  == True)
	{
		_Connection->m_HasReaded = False;
		int readed = Buffer_WriteBuffer(_Buffer, _Connection->m_Buffer.m_ReadPtr, _Connection->m_Buffer.m_BytesLeft);
		Buffer_Clear(&_Connection->m_Buffer);
		return readed;
	}

	return 0;
}

int Filesystem_Connection_OnWrite(void* _Context, Buffer* _Buffer)
{
	Filesystem_Connection* _Connection = (Filesystem_Connection*)_Context;

	void* ptr = _Buffer->m_ReadPtr;
	int bytesLeft = _Buffer->m_BytesLeft;
	int bytesWrite = 0;
	int totalWrite = 0;
	int bytesToWrite = 0;
	unsigned char dataBuffer[TCP_BufferSize];

	while(bytesLeft > 0)
	{
		bytesToWrite = bytesLeft;
		if(bytesToWrite > TCP_BufferSize - totalWrite)
			bytesToWrite = TCP_BufferSize - totalWrite;
		else if(bytesToWrite > TCP_BufferSize)
			bytesToWrite = TCP_BufferSize;

		Memory_Copy(dataBuffer, ptr, bytesToWrite);
		TCPSocket_Error error;
		bytesWrite = TCPSocket_Write(_Connection->m_Socket, dataBuffer, bytesToWrite, &error);
		if(error.m_HasError == True)
		{
			switch (error.m_Error)
			{
				case 0:
				case EWOULDBLOCK:
				{
					bytesWrite = 0;
				} break;
				case 32: //* Broken pipe
				{
					EventHandler_EventCall(&_Connection->m_EventHandler, Filesystem_Connection_Event_Disconnected, _Connection);
					return -1;
				} break;

				default:
				{
					return -2;
				} break;
			}
		}

		bytesLeft -= bytesWrite;
		totalWrite += bytesWrite;
		ptr += bytesWrite;
	}
	
	_Buffer->m_ReadPtr += _Buffer->m_BytesLeft;
	_Buffer->m_BytesLeft = 0;

	return totalWrite;
}

int Connection_Reconnect(Filesystem_Connection* _Connection)
{
	
	char ip[16];
	Payload_GetIP(&_Connection->m_Addrass, ip);
	printf("TODO reconnect to \"%s:%i\"\r\n", ip, _Connection->m_Port);

	EventHandler_EventCall(&_Connection->m_EventHandler, Filesystem_Connection_Event_ReconnectError, _Connection);

	return 0;
}

void Filesystem_Connection_Dispose(Filesystem_Connection* _Connection)
{
	if(_Connection->m_Disposed == True)
		return;
	
	_Connection->m_Disposed = True;
	EventHandler_EventCall(&_Connection->m_EventHandler, Filesystem_Connection_Event_Disposed, _Connection);

	if(_Connection->m_Addrass.m_Type == Payload_Address_Type_IP)
	{
		char ip[16];
		Payload_GetIP(&_Connection->m_Addrass, ip);
		printf("Connection \"%s:%i\" got dispose\r\n", ip, _Connection->m_Port);

	}
	else if(_Connection->m_Addrass.m_Type == Payload_Address_Type_MAC)
	{
		char mac[18];
		Payload_GetMac(&_Connection->m_Addrass, mac);
		printf("Connection \"%s:%i\" got dispose\r\n", mac, _Connection->m_Port);
	}

	Bus_RemoveFuncIn(_Connection->m_Bus, _Connection->m_Func);
	Buffer_Dispose(&_Connection->m_Buffer);

	EventHandler_Dispose(&_Connection->m_EventHandler);
	TCPSocket_Dispose(_Connection->m_Socket);
	StateMachine_RemoveTask(_Connection->m_Worker, _Connection->m_Task);

	if(_Connection->m_Allocated == True)
	{
		Allocator_Free(_Connection);
	}
	else
	{
		memset(_Connection, 0, sizeof(Filesystem_Connection));
		_Connection->m_Disposed = True;
	}
}