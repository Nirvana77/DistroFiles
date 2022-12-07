#ifndef Bus_h__
#define Bus_h__

struct T_Bus;
typedef struct T_Bus Bus;

#include "../Types.h"
#include "../LinkedList.h"
#include "Payload.h"
#include "../EventHandler.h"

struct T_Bus_Function;
typedef struct T_Bus_Function Bus_Function;

#ifndef BUS_BUFFER_SIZE
	#define BUS_BUFFER_SIZE 64
#endif


struct T_Bus
{
	Bool m_Allocated;
	LinkedList m_FuncIn;
	LinkedList m_FuncOut;

	Buffer m_ReadBuffer;
	Buffer m_WriteBuffer;

	EventHandler m_EventHandler;
};

int Bus_InitializePtr(Bus** _BusPtr);
int Bus_Initialize(Bus* _Bus);

int Bus_AddFuncIn(Bus* _Bus, int (*_OnRead)(void* _Context, Buffer* _Buffer), int (*_OnWrite)(void* _Context, Buffer* _Buffer), void* _Context, Payload_FuncIn** _FuncPtr);

//! This is not implemented
int Bus_AddFuncOut(Bus* _Bus, int (*_OnRead)(void* _Context, Buffer* _Buffer), int (*_OnWrite)(void* _Context, Buffer* _Buffer), void* _Context, Payload_FuncIn** _FuncPtr);

int Bus_RemoveFuncIn(Bus* _Bus, Payload_FuncIn* _Func);
//! This is not implemented
int Bus_RemoveFuncOut(Bus* _Bus, Payload_FuncIn* _Func);

int Bus_OnRead(void* _Context, Buffer* _Buffer);
int Bus_OnWrite(void* _Context, Buffer* _Buffer);

void Bus_Dispose(Bus* _Bus);
#endif // Bus_h__
