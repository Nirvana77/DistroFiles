#include "StateMachine.h"

int StateMachine_InitializePtr(StateMachine** _StateMachinePtr)
{
	StateMachine* _StateMachine = (StateMachine*)Allocator_Malloc(sizeof(StateMachine));
	if(_StateMachine == NULL)
		return -1;
	
	int success = StateMachine_Initialize(_StateMachine);
	if(success != 0)
	{
		Allocator_Free(_StateMachine);
		return success;
	}
	
	_StateMachine->m_Allocated = True;
	
	*(_StateMachinePtr) = _StateMachine;
	return 0;
}

int StateMachine_Initialize(StateMachine* _StateMachine)
{
	_StateMachine->m_Allocated = False;

	LinkedList_Initialize(&_StateMachine->m_List);
	
	return 0;
}

//TODO: fix prio.
int StateMachine_CreateTask(StateMachine* _StateMachine, unsigned int _Prio, const char* _Name, void (*_Callback)(u_int32_t _MSTime, void* _Context), void* _Context, StateMachine_Task** _TaskPtr)
{
	StateMachine_Task* _Task = (StateMachine_Task*) Allocator_Malloc(sizeof(StateMachine_Task));

	if(_Task == NULL)
		return -1;

	_Task->m_Callback = _Callback;
	_Task->m_Context = _Context;
	_Task->m_Prio = _Prio;

	if(LinkedList_Push(&_StateMachine->m_List, _Task) != 0)
	{
		Allocator_Free(_Task);
		return -2;
	}

	if(_StateMachine->m_Current == NULL)
		_StateMachine->m_Current = _StateMachine->m_List.m_Head;

	//TODO: resort list with prio
	
	if(_TaskPtr != NULL)
		*(_TaskPtr) = _Task;

	return 0;
}

void StateMachine_Work(StateMachine* _StateMachine)
{
	if(_StateMachine->m_Current == NULL)
		return;

	StateMachine_Task* _Task = (StateMachine_Task*) _StateMachine->m_Current->m_Item;
	_StateMachine->m_Current = _StateMachine->m_Current->m_Next;

	if(_StateMachine->m_Current == NULL)
		_StateMachine->m_Current = _StateMachine->m_List.m_Head;
	long ms; 
	time_t s; 

	struct timespec spec;
	u_int64_t timeMS = 0;
	clock_gettime(CLOCK_REALTIME, &spec);
	s  = spec.tv_sec;
	ms = (spec.tv_nsec / 1.0e6);

	timeMS = s;
	timeMS *= 1000;
	timeMS += ms;
	if(_Task->m_Callback != NULL)
		_Task->m_Callback(timeMS, _Task->m_Context);

}

void StateMachine_Dispose(StateMachine* _StateMachine)
{
	LinkedList_Node* currentNode = _StateMachine->m_List.m_Head;

	while (currentNode != NULL)
	{
		StateMachine_Task* task = (StateMachine_Task*) currentNode->m_Item;
		currentNode = currentNode->m_Next;
		LinkedList_RemoveFirst(&_StateMachine->m_List);
		Allocator_Free(task);
	}

	LinkedList_Dispose(&_StateMachine->m_List);
	

	if(_StateMachine->m_Allocated == True)
		Allocator_Free(_StateMachine);
	else
		memset(_StateMachine, 0, sizeof(StateMachine));

}