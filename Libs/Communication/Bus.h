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

#ifndef BUS_DATABUFFER_SIZE
	#define BUS_DATABUFFER_SIZE 255
#endif

struct T_Bus_Function
{
	void* m_Context;
	int (*m_OnRead)(void* _Context, Buffer* _Buffer);
	int (*m_OnWrite)(void* _Context, Buffer* _Buffer);
};

struct T_Bus
{
	Bool m_Allocated;
	LinkedList m_FuncIn;
	LinkedList m_FuncOut;

	EventHandler m_EventHandler;
};

int Bus_InitializePtr(Bus** _BusPtr);
int Bus_Initialize(Bus* _Bus);

int Bus_AddFunction(Bus* _Bus, int (*_OnRead)(void* _Context, Buffer* _Buffer), void* _ReadContext, int (*_OnWrite)(void* _Context, Buffer* _Buffer), void* _WriteContext);


int Bus_Read(void* _Context, Buffer* _Buffer);
int Bus_Write(void* _Context, Buffer* _Buffer);

void Bus_Dispose(Bus* _Bus);
#endif // Bus_h__
