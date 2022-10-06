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
	String m_IP;
	int m_Port;

	TCPSocket m_Socket;
	struct sockaddr_in m_Addr;

};

int TCPServer_InitializePtr(const char* _IP, int _Port, TCPServer** _TCPServerPtr);
int TCPServer_Initialize(TCPServer* _TCPServer, const char* _IP, int _Port);

int TCPServer_Listen(TCPServer* _TCPServer);

void TCPServer_Disconnect(TCPServer* _TCPServer);

void TCPServer_Dispose(TCPServer* _TCPServer);

#endif // TCPServer_h__