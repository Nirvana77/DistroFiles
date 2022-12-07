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
	Buffer_Initialize(&_Bus->m_ReadBuffer, BUS_BUFFER_SIZE);
	Buffer_Initialize(&_Bus->m_WriteBuffer, BUS_BUFFER_SIZE);
	EventHandler_Initialize(&_Bus->m_EventHandler);
	
	return 0;
}

int Bus_AddFuncIn(Bus* _Bus, int (*_OnRead)(void* _Context, Buffer* _Buffer), int (*_OnWrite)(void* _Context, Buffer* _Buffer), void* _Context, Payload_FuncIn** _FuncPtr)
{
	if(_OnRead == NULL && _OnWrite == NULL)
		return -2;

	Payload_FuncIn* _Func = (Payload_FuncIn*)Allocator_Malloc(sizeof(Payload_FuncIn));
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

	if(_FuncPtr != NULL)
		*(_FuncPtr) = _Func;

	return 0;
}

int Bus_AddFuncOut(Bus* _Bus, int (*_OnRead)(void* _Context, Buffer* _Buffer), int (*_OnWrite)(void* _Context, Buffer* _Buffer), void* _Context, Payload_FuncIn** _FuncPtr)
{
	if(_OnRead == NULL && _OnWrite == NULL)
		return -2;

	Payload_FuncIn* _Func = (Payload_FuncIn*)Allocator_Malloc(sizeof(Payload_FuncIn));
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

	if(_FuncPtr != NULL)
		*(_FuncPtr) = _Func;

	return 0;
}

int Bus_RemoveFuncIn(Bus* _Bus, Payload_FuncIn* _Func)
{
	int success = LinkedList_RemoveItem(&_Bus->m_FuncIn, _Func);
	if(success == 0)
		Allocator_Free(_Func);

	return success;
}

int Bus_RemoveFuncOut(Bus* _Bus, Payload_FuncIn* _Func)
{
	int success = LinkedList_RemoveItem(&_Bus->m_FuncOut, _Func);
	if(success == 0)
		Allocator_Free(_Func);

	return success;
}

int Bus_OnRead(void* _Context, Buffer* _Buffer)
{
	Bus* _Bus = (Bus*) _Context;
	int totalReaded = 0;

	LinkedList_Node* currentNode = _Bus->m_FuncIn.m_Head;
	while(currentNode != NULL)
	{
		Payload_FuncIn* _Func = (Payload_FuncIn*) currentNode->m_Item;
		currentNode = currentNode->m_Next;

		if(_Func->m_OnRead != NULL)
			totalReaded += _Func->m_OnRead(_Func->m_Context, _Buffer);
		

	}

	return totalReaded;
}

int Bus_OnWrite(void* _Context, Buffer* _Buffer)
{
	Bus* _Bus = (Bus*) _Context;

	LinkedList_Node* currentNode = _Bus->m_FuncIn.m_Head;
	while(currentNode != NULL)
	{
		Payload_FuncIn* _Func = (Payload_FuncIn*) currentNode->m_Item;
		currentNode = currentNode->m_Next;

		if(_Func->m_OnWrite != NULL)
		{
			Buffer_ResetReadPtr(_Buffer);
			int writed = _Func->m_OnWrite(_Func->m_Context, _Buffer);

			if(writed < 0)
			{
				printf("Bus writed error: %i\r\n", writed);
				return -1;
			}
		}
		

	}

	return 0;
}

void Bus_Dispose(Bus* _Bus)
{
	LinkedList_Node* currentNode = _Bus->m_FuncIn.m_Head;
	while(currentNode != NULL)
	{
		Payload_FuncIn* _Func = (Payload_FuncIn*) currentNode->m_Item;
		currentNode = currentNode->m_Next;
		LinkedList_RemoveFirst(&_Bus->m_FuncIn);
		Allocator_Free(_Func);
	}

	currentNode = _Bus->m_FuncOut.m_Head;
	while(currentNode != NULL)
	{
		Payload_FuncIn* _Func = (Payload_FuncIn*) currentNode->m_Item;
		currentNode = currentNode->m_Next;
		LinkedList_RemoveFirst(&_Bus->m_FuncOut);
		Allocator_Free(_Func);
	}

	EventHandler_Dispose(&_Bus->m_EventHandler);
	Buffer_Dispose(&_Bus->m_ReadBuffer);
	Buffer_Dispose(&_Bus->m_WriteBuffer);

	if(_Bus->m_Allocated == True)
		Allocator_Free(_Bus);
	else
		memset(_Bus, 0, sizeof(Bus));

}