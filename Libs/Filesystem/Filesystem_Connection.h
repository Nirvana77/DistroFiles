#ifndef Filesystem_Connection_h__
#define Filesystem_Connection_h__

struct T_Filesystem_Connection;
typedef struct T_Filesystem_Connection Filesystem_Connection;

#include "Filesystem_Service.h"

#define Filesystem_Connection_Timeout MIN*10

typedef enum
{
	Filesystem_Connection_Event_Readed = 0,
	Filesystem_Connection_Event_Disposed = 1,
	Filesystem_Connection_Event_Disconnected = 2,

} Filesystem_Connection_Event;

struct T_Filesystem_Connection
{
	Bool m_Allocated;

	Payload_Address m_Addrass;
	TCPSocket* m_Socket;
	StateMachine* m_Worker;
	Buffer* m_Buffer;

	UInt64 m_NextCheck;

	EventHandler m_EventHandler;

	StateMachine_Task* m_Task;
};

int Filesystem_Connection_InitializePtr(StateMachine* _Worker, TCPSocket* _Socket, Buffer* _Buffer, Filesystem_Connection** _CommectionPtr);
int Filesystem_Connection_Initialize(Filesystem_Connection* _Connection, StateMachine* _Worker, TCPSocket* _Socket, Buffer* _Buffer);


void Filesystem_Connection_Dispose(Filesystem_Connection* _Connection);
#endif // Filesystem_Connection_h__