#ifndef DistroFiles_Connection_h__
#define DistroFiles_Connection_h__

struct T_DistroFiles_Connection;
typedef struct T_DistroFiles_Connection DistroFiles_Connection;

#include "DistroFiles_Service.h"
#include "../Communication/Bus.h"

#define DistroFiles_Connection_Timeout (SEC*10)

#ifndef TCP_BufferSize
	#define TCP_BufferSize 256
#endif

typedef enum
{
	DistroFiles_Connection_Event_Readed = 0,
	DistroFiles_Connection_Event_Disposed = 1,
	DistroFiles_Connection_Event_Disconnected = 2,
	DistroFiles_Connection_Event_Reconnected = 3,
	DistroFiles_Connection_Event_ReconnectError = 4,

} DistroFiles_Connection_Event;

struct T_DistroFiles_Connection
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

int DistroFiles_Connection_InitializePtr(StateMachine* _Worker, TCPSocket* _Socket, Bus* _Bus, DistroFiles_Connection** _CommectionPtr);
int DistroFiles_Connection_Initialize(DistroFiles_Connection* _Connection, StateMachine* _Worker, TCPSocket* _Socket, Bus* _Bus);

int Connection_Reconnect(DistroFiles_Connection* _Connection);

void DistroFiles_Connection_Dispose(DistroFiles_Connection* _Connection);
#endif // DistroFiles_Connection_h__