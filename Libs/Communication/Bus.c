#include "Bus.h"

int Bus_InitializePtr(Bus** _BusPtr)
{
	Bus* _Bus = (Bus*)Allocator_Malloc(sizeof(Bus));
	if(_Bus == NULL)
		return -1;
	
	int success = Bus_Initialize(_Bus);
	if(success != 0)
	{
		Allocator_Free(_Bus);
		return success;
	}
	
	_Bus->m_Allocated = True;
	
	*(_BusPtr) = _Bus;
	return 0;
}

int Bus_Initialize(Bus* _Bus)
{
	_Bus->m_Allocated = False;

	LinkedList_Initialize(&_Bus->m_FuncIn);
	LinkedList_Initialize(&_Bus->m_FuncOut);
	EventHandler_Initialize(&_Bus->m_EventHandler);
	
	return 0;
}

int Bus_AddFuncIn(Bus* _Bus, int (*_OnRead)(void* _Context, Buffer* _Buffer), int (*_OnWrite)(void* _Context, Buffer* _Buffer), void* _Context)
{
	if(_OnRead == NULL && _OnWrite == NULL)
		return -2;

	Bus_Function* _Func = (Bus_Function*)Allocator_Malloc(sizeof(Bus_Function));
	if(_Func == NULL)
		return -1;

	_Func->m_Context = _Context;
	_Func->m_OnRead = _OnRead;
	_Func->m_OnWrite = _OnWrite;

	if(LinkedList_Push(&_Bus->m_FuncIn, _Func) != 0)
	{
		Allocator_Free(_Func);
		return -3;
	}

	return 0;
}
int Bus_AddFuncOut(Bus* _Bus, int (*_OnRead)(void* _Context, Buffer* _Buffer), int (*_OnWrite)(void* _Context, Buffer* _Buffer), void* _Context)
{
	if(_OnRead == NULL && _OnWrite == NULL)
		return -2;

	Bus_Function* _Func = (Bus_Function*)Allocator_Malloc(sizeof(Bus_Function));
	if(_Func == NULL)
		return -1;

	_Func->m_Context = _Context;
	_Func->m_OnRead = _OnRead;
	_Func->m_OnWrite = _OnWrite;

	if(LinkedList_Push(&_Bus->m_FuncOut, _Func) != 0)
	{
		Allocator_Free(_Func);
		return -3;
	}
	return 0;
}

int Bus_Read(void* _Context, Buffer* _Buffer)
{
	Bus* _Bus = (Bus*)_Context;

	int totalReaded = 0;

	LinkedList_Node* currentNode = _Bus->m_FuncIn.m_Head;
	while(currentNode != NULL)
	{
		Bus_Function* _Func = (Bus_Function*) currentNode->m_Item;
		currentNode = currentNode->m_Next;
		
		if(_Func->m_OnRead != NULL)
		{
			int size = Buffer_SizeLeft(_Buffer);
			int readed = _Func->m_OnRead(_Func->m_Context, _Buffer);
			totalReaded += readed;
			while (readed == size)
			{
				Buffer_Extend(_Buffer);
				size = Buffer_SizeLeft(_Buffer);
				readed = _Func->m_OnRead(_Func->m_Context, _Buffer);
				totalReaded += readed;
			}


		}

	}

	return totalReaded;

}

int Bus_Write(void* _Context, Buffer* _Buffer)
{
	Bus* _Bus = (Bus*)_Context;

	LinkedList_Node* currentNode = _Bus->m_FuncIn.m_Head;
	int success = 0;
	while(currentNode != NULL)
	{
		Bus_Function* _Func = (Bus_Function*) currentNode->m_Item;
		currentNode = currentNode->m_Next;

		if(_Func->m_OnWrite != NULL)
		{
			if(_Func->m_OnWrite(_Func->m_Context, _Buffer) != 0)
				success--;
		}
	}

	return success;
}

void Bus_Dispose(Bus* _Bus)
{
	LinkedList_Node* currentNode = _Bus->m_FuncIn.m_Head;
	while(currentNode != NULL)
	{
		Bus_Function* _Func = (Bus_Function*) currentNode->m_Item;
		currentNode = currentNode->m_Next;
		LinkedList_RemoveFirst(&_Bus->m_FuncIn);
		Allocator_Free(_Func);
	}

	currentNode = _Bus->m_FuncOut.m_Head;
	while(currentNode != NULL)
	{
		Bus_Function* _Func = (Bus_Function*) currentNode->m_Item;
		currentNode = currentNode->m_Next;
		LinkedList_RemoveFirst(&_Bus->m_FuncOut);
		Allocator_Free(_Func);
	}

	EventHandler_Dispose(&_Bus->m_EventHandler);

	if(_Bus->m_Allocated == True)
		Allocator_Free(_Bus);
	else
		memset(_Bus, 0, sizeof(Bus));

}