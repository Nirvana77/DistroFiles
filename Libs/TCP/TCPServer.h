#ifndef TCPServer_h__
#define TCPServer_h__

struct T_TCPServer;
typedef struct T_TCPServer TCPServer;

#include <unistd.h>
#include <arpa/inet.h>

#include "../String.h"
#include "TCPSocket.h"

struct T_TCPServer
{
	Bool m_Allocated;

	TCPSocket_FD m_Socket;
	struct sockaddr_in m_ServerAddr;

	int (*m_ConnectedSocketClackkback)(TCPSocket* _TCPSocket, void* _Context);
	void* m_Context;

};

int TCPServer_InitializePtr(int (*m_ConnectedSocketClackkback)(TCPSocket* _TCPSocket, void* _Context), void* _Context, TCPServer** _TCPServerPtr);
int TCPServer_Initialize(TCPServer* _TCPServer, int (*m_ConnectedSocketClackkback)(TCPSocket* _TCPSocket, void* _Context), void* _Context);

void TCPServer_Work(TCPServer* _TCPServer);

int TCPServer_Listen(TCPServer* _TCPServer, const char* _IP, UInt16 _Port);

void TCPServer_Disconnect(TCPServer* _TCPServer);

void TCPServer_Dispose(TCPServer* _TCPServer);

#endif // TCPServer_h__