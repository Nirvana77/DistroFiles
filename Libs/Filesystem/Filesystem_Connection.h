#ifndef Filesystem_Connection_h__
#define Filesystem_Connection_h__

struct T_Filesystem_Connection;
typedef struct T_Filesystem_Connection Filesystem_Connection;

#include "Filesystem_Service.h"
#include "../Communication/Bus.h"

#define Filesystem_Connection_Timeout (SEC*10)

#ifndef TCP_BufferSize
	#define TCP_BufferSize 256
#endif

typedef enum
{
	Filesystem_Connection_Event_Readed = 0,
	Filesystem_Connection_Event_Disposed = 1,
	Filesystem_Connection_Event_Disconnected = 2,

} Filesystem_Connection_Event;

struct T_Filesystem_Connection
{
	Bool m_Allocated;
	Bool m_Disposed;

	Payload_Address m_Addrass;
	UInt16 m_Port;
	
	TCPSocket* m_Socket;
	StateMachine* m_Worker;
	Bus* m_Bus;

	Buffer m_Buffer;
	unsigned char m_DataBuffer[TCP_BufferSize];
	Bool m_HasReaded;
	Payload_FuncIn* m_Func;

	UInt64 m_NextCheck;

	EventHandler m_EventHandler;

	StateMachine_Task* m_Task;
};

int Filesystem_Connection_InitializePtr(StateMachine* _Worker, TCPSocket* _Socket, Bus* _Bus, Filesystem_Connection** _CommectionPtr);
int Filesystem_Connection_Initialize(Filesystem_Connection* _Connection, StateMachine* _Worker, TCPSocket* _Socket, Bus* _Bus);


void Filesystem_Connection_Dispose(Filesystem_Connection* _Connection);
#endif // Filesystem_Connection_h__