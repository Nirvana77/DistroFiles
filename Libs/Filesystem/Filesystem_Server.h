#ifndef Filesystem_Server_h__
#define Filesystem_Server_h__

struct T_Filesystem_Server;
typedef struct T_Filesystem_Server Filesystem_Server;

#include "Filesystem_Service.h"

struct T_Filesystem_Server
{
	Bool m_Allocated;
	Filesystem_Service* m_Service;

	TCPServer m_TCPServer;
	DataLayer m_DataLayer;
	TransportLayer m_TransportLayer;
	
	LinkedList m_Sockets;
	LinkedList_Node* m_CurrentNode;
	Buffer m_Buffer;
	
};

int Filesystem_Server_InitializePtr(Filesystem_Service* _Service, Filesystem_Server** _ServerPtr);
int Filesystem_Server_Initialize(Filesystem_Server* _Server, Filesystem_Service* _Service);

void Filesystem_Server_Work(UInt64 _MSTime, Filesystem_Server* _Server);

void Filesystem_Server_Dispose(Filesystem_Server* _Server);

#endif // Filesystem_Server_h__
