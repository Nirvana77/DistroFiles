#include "StateMachine.h"

void* StateMachine_TaskWork(void* _Context);
void StateMachine_DisposeThread(StateMachine_Task* _Task);
void* StateMachine_InternalDisposeThread(void* _Context);


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

int StateMachine_CreateTask(StateMachine* _StateMachine, pthread_attr_t* _Attr, int (*_Callback)(UInt64 _MSTime, void* _Context), void* _Context, StateMachine_Task** _TaskPtr)
{
	if(_Callback == NULL)
		return -2;

	StateMachine_Task* _Task = (StateMachine_Task*) Allocator_Malloc(sizeof(StateMachine_Task));

	if(_Task == NULL)
		return -1;
	
	_Task->m_Disposed = False;

	_Task->m_Callback = _Callback;
	_Task->m_Context = _Context;

    int success = pthread_create(&_Task->m_Thread, _Attr, &StateMachine_TaskWork, _Task);
	if(success != 0)
	{
		Allocator_Free(_Task);
		return -3;
	}

	if(LinkedList_Push(&_StateMachine->m_List, _Task) != 0)
	{
		StateMachine_DisposeThread(_Task);
		Allocator_Free(_Task);
		return -4;
	}
	
	if(_TaskPtr != NULL)
		*(_TaskPtr) = _Task;

	return 0;
}

int StateMachine_RemoveTask(StateMachine* _StateMachine, StateMachine_Task* _Task)
{
	LinkedList_Node* currentNode = _StateMachine->m_List.m_Head;

	while (currentNode != NULL)
	{
		StateMachine_Task* task = (StateMachine_Task*) currentNode->m_Item;
		if(_Task == task && task->m_Disposed == False)
		{
			StateMachine_DisposeThread(task);
			LinkedList_RemoveNode(&_StateMachine->m_List, currentNode);
			Allocator_Free(task);
			return 0;
		}
		currentNode = currentNode->m_Next;
	}

	return 1;
}

void* StateMachine_TaskWork(void* _Context)
{
	StateMachine_Task* _Task = (StateMachine_Task*) _Context;
	while (_Task->m_Disposed == False)
	{
		UInt64 monoTime = 0;

		SystemMonotonicMS(&monoTime);

		int willDispose = _Task->m_Callback(monoTime, _Task->m_Context);
		if(willDispose == 1)
		{
			pthread_t thread;
			pthread_create(&thread, NULL, &StateMachine_InternalDisposeThread, _Task);
			sleep(1);
		}
	}

	return NULL;
}

void StateMachine_DisposeThread(StateMachine_Task* _Task)
{
	_Task->m_Disposed = True;
	pthread_join(_Task->m_Thread, NULL);
}

void* StateMachine_InternalDisposeThread(void* _Context)
{
	StateMachine_Task* _Task = (StateMachine_Task*) _Context;
	StateMachine_DisposeThread(_Task);

	return NULL;
}

void StateMachine_Dispose(StateMachine* _StateMachine)
{
	LinkedList_Node* currentNode = _StateMachine->m_List.m_Head;

	while (currentNode != NULL)
	{
		StateMachine_Task* task = (StateMachine_Task*) currentNode->m_Item;
		currentNode = currentNode->m_Next;

		StateMachine_DisposeThread(task);
		LinkedList_RemoveFirst(&_StateMachine->m_List);
		Allocator_Free(task);
	}

	LinkedList_Dispose(&_StateMachine->m_List);
	

	if(_StateMachine->m_Allocated == True)
		Allocator_Free(_StateMachine);
	else
		memset(_StateMachine, 0, sizeof(StateMachine));

}