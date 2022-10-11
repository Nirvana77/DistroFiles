#ifndef TCPClient_h__
#define TCPClient_h__

struct T_TCPClient;
typedef struct T_TCPClient TCPClient;

#include "TCPSocket.h"
#include "../Allocator.h"
#include "../String.h"

typedef enum
{
	TCPClient_State_Error = -1,
	TCPClient_State_Init = 0,
	TCPClient_State_Connected = 1,
	TCPClient_State_Disconnected = 2,
} TCPClient_State;

struct T_TCPClient
{
	Bool m_Allocated;

	String m_Host;
	UInt16 m_Port;

	TCPClient_State m_State;

	TCPSocket* m_Socket;

};

int TCPClient_InitializePtr(const char* _Host, UInt16 _Port, TCPClient** _TCPClientPtr);
int TCPClient_Initialize(TCPClient* _TCPClient, const char* _Host, UInt16 _Port);

int TCPClient_Read(void* _TCPClient, Buffer* _Buffer, int _Size);

int TCPClient_Write(void* _TCPClient, Buffer* _Buffer, int _Size);

void TCPClient_Disconnect(TCPClient* _TCPClient);

void TCPClient_Dispose(TCPClient* _TCPClient);

#endif // TCPClient_h__