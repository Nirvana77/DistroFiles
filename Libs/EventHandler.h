#ifndef EventHandler_h__
#define EventHandler_h__

struct T_EventHandler;
typedef struct T_EventHandler EventHandler;

struct T_EventHandler_Event;
typedef struct T_EventHandler_Event EventHandler_Event;

#include "Allocator.h"
#include "LinkedList.h"

struct T_EventHandler_Event
{
	void* m_Context;
	int (*m_Callback)(EventHandler* _EventHandler, int _EventCall, void* _Context);
};
struct T_EventHandler
{
	Bool m_Allocated;
	LinkedList m_Events;
};
int EventHandler_InitializePtr(EventHandler** _EventHandlerPtr);
int EventHandler_Initialize(EventHandler* _EventHandler);

void EventHandler_Dispose(EventHandler* _EventHandler);

#endif // EventHandler_h__