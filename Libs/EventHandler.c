#include "EventHandler.h"

int EventHandler_InitializePtr(EventHandler** _EventHandlerPtr)
{
	EventHandler* _EventHandler = (EventHandler*)Allocator_Malloc(sizeof(EventHandler));
	if(_EventHandler == NULL)
		return -1;
	
	int success = EventHandler_Initialize(_EventHandler);
	if(success != 0)
	{
		Allocator_Free(_EventHandler);
		return success;
	}
	
	_EventHandler->m_Allocated = True;
	
	*(_EventHandlerPtr) = _EventHandler;
	return 0;
}

int EventHandler_Initialize(EventHandler* _EventHandler)
{
	_EventHandler->m_Allocated = False;

	LinkedList_Initialize(&_EventHandler->m_Events);
	
	return 0;
}

EventHandler_Event* EventHandler_Hook(EventHandler* _EventHandler, int (*_Callback)(EventHandler* _EventHandler, int _EventCall, void* _Object, void* _Context), void* _Context)
{
	EventHandler_Event* _Event = (EventHandler_Event*) Allocator_Malloc(sizeof(EventHandler_Event));

	if(_Event == NULL)
		return NULL;
	
	_Event->m_Callback = _Callback;
	_Event->m_Context = _Context;

	LinkedList_Push(&_EventHandler->m_Events, _Event);

	return _Event;
}


int EventHandler_UnHook(EventHandler* _EventHandler, EventHandler_Event* _Event)
{
	if(LinkedList_RemoveItem(&_EventHandler->m_Events, _Event) == 0)
		Allocator_Free(_Event);
	return 0;
}

void EventHandler_EventCall(EventHandler* _EventHandler, int _EventCall, void* _Object)
{

	LinkedList_Node* currentNode = _EventHandler->m_Events.m_Head;
	while (currentNode != NULL)
	{
		EventHandler_Event* _Event = (EventHandler_Event*)currentNode->m_Item;
		currentNode = currentNode->m_Next;

		int willUnHook = 0;
		if(_Event->m_Callback != NULL)
			willUnHook = _Event->m_Callback(_EventHandler, _EventCall, _Object, _Event->m_Context);
		
		if(willUnHook == 1)
			EventHandler_UnHook(_EventHandler, _Event);
		
		
	}

}


void EventHandler_Dispose(EventHandler* _EventHandler)
{
	LinkedList_Node* currentNode = _EventHandler->m_Events.m_Head;
	while (currentNode != NULL)
	{
		EventHandler_Event* _Event = (EventHandler_Event*)currentNode->m_Item;
		currentNode = currentNode->m_Next;
		LinkedList_RemoveFirst(&_EventHandler->m_Events);
		Allocator_Free(_Event);
	}
	

	if(_EventHandler->m_Allocated == True)
		Allocator_Free(_EventHandler);
	else
		memset(_EventHandler, 0, sizeof(EventHandler));

}