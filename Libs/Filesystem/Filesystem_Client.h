#ifndef Filesystem_Client_h__
#define Filesystem_Client_h__

struct T_Filesystem_Client;
typedef struct T_Filesystem_Client Filesystem_Client;

#include "../String.h"
#include "../File.h"
#include "../Folder.h"
#include "../Json.h"
#include "../Portability.h"
#include "../StateMachine.h"
#include "../TCP/TCPClient.h"
#include "../Buffer.h"

struct T_Filesystem_Client
{
	Bool m_Allocated;
	StateMachine* m_Worker;

	TCPClient m_TCPClient;
	TCPSocket m_ServerSocket;

};

int Filesystem_Client_InitializePtr(StateMachine* _Worker, Filesystem_Client** _ClientPtr);
int Filesystem_Client_Initialize(Filesystem_Client* _Client, StateMachine* _Worker);

void Filesystem_Client_Work(UInt64 _MSTime, void* _Context);

void Filesystem_Client_Dispose(Filesystem_Client* _Client);
#endif // Filesystem_Client_h__