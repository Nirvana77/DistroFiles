#include "TransportLayer.h"

int TransportLayer_InitializePtr(TransportLayer** _TransportLayerPtr)
{
	TransportLayer* _TransportLayer = (TransportLayer*)Allocator_Malloc(sizeof(TransportLayer));
	if(_TransportLayer == NULL)
		return -1;
	
	int success = TransportLayer_Initialize(_TransportLayer);
	if(success != 0)
	{
		Allocator_Free(_TransportLayer);
		return success;
	}
	
	_TransportLayer->m_Allocated = True;
	
	*(_TransportLayerPtr) = _TransportLayer;
	return 0;
}

int TransportLayer_Initialize(TransportLayer* _TransportLayer)
{
	_TransportLayer->m_Allocated = False;

	int success = LinkedList_Initialize(&_TransportLayer->m_Queued);
	if(success != 0)
	{
		printf("Failed to initialize the Queued!\n\r");
		printf("Error code: %i\n\r", success);
		return -2;
	}
	success = LinkedList_Initialize(&_TransportLayer->m_Sented);
	if(success != 0)
	{
		printf("Failed to initialize the Sented!\n\r");
		printf("Error code: %i\n\r", success);
		return -2;
	}
	
	return 0;
}

int TransportLayer_CreateMessage(TransportLayer* _TransportLayer, Payload_Type _Type, int _Size, int _Timeout, Payload** _PayloadPtr)
{
	if(_Size == 0)
		return -2;

	Payload* _Payload = NULL;

	if(Payload_InitializePtr(NULL, &_Payload) != 0)
		return -1;

	LinkedList_Push(&_TransportLayer->m_Queued, _Payload);
	
	_Payload->m_Size = _Size;
	_Payload->m_Timeout = _Timeout;

	_Payload->m_Type = _Type;



	if(_PayloadPtr != NULL)
		*(_PayloadPtr) = _Payload;

	return 0;
}

int TransportLayer_DestroyMessage(TransportLayer* _TransportLayer, Payload* _Payload)
{
	int success = LinkedList_RemoveItem(&_TransportLayer->m_Queued, _Payload);

	if(success < 0)
		return -1;

	if(success == 1)
		return 1;

	Payload_Dispose(_Payload);

	return 0;
}

int TransportLayer_RemoveMessage(TransportLayer* _TransportLayer, Payload* _Payload)
{
	return LinkedList_RemoveItem(&_TransportLayer->m_Queued, _Payload);
}

//TODO #17 Fix this to be a propper transport layer
int TransportLayer_SendPayload(void* _Context, Payload** _PaylodePtr)
{
	TransportLayer* _TransportLayer = (TransportLayer*) _Context;

	if(_TransportLayer->m_FuncOut.m_Send != NULL)
	{
		if(_TransportLayer->m_FuncOut.m_Send(_TransportLayer->m_FuncOut.m_Context, _PaylodePtr) == 1)
		{
			printf("TransportLayer_SendPayload error\n\r");
			printf("Not implemented\r\n");
			return 1;
		}
	}
	else if(_TransportLayer->m_Queued.m_Size > 0)
	{
		Payload* p = (Payload*)LinkedList_RemoveFirst(&_TransportLayer->m_Queued);
		p->m_State = Payload_State_Sending;
		SystemMonotonicMS(&p->m_Time);

		LinkedList_Push(&_TransportLayer->m_Sented, p);

		*(_PaylodePtr) = p;
		
		return 1;
 	}


	return 0;
}

int TransportLayer_ReveicePayload(void* _Context, Payload* _Message, Payload* _Replay)
{
	TransportLayer* _TransportLayer = (TransportLayer*) _Context;


	if(_TransportLayer->m_FuncOut.m_Receive != NULL)
	{
		Payload* replay;
		Payload_InitializePtr(_Replay->m_UUID, &replay);
		if(_TransportLayer->m_FuncOut.m_Receive(_TransportLayer->m_FuncOut.m_Context, _Message, replay) == 1)
		{
			
			Payload_FilCommunicator(&replay->m_Des, &_Message->m_Src);
			LinkedList_Push(&_TransportLayer->m_Queued, replay);

			return 0;
		}
		Payload_Dispose(replay);
	}

	return 0;
}


void TransportLayer_Work(UInt64 _MSTime, TransportLayer* _TransportLayer)
{
	LinkedList_Node* currentNode = _TransportLayer->m_Sented.m_Head;
	while(currentNode != NULL)
	{
		Payload* _Payload = (Payload*) currentNode->m_Item;
		currentNode = currentNode->m_Next;
		
		if(_MSTime > _Payload->m_Timeout + _Payload->m_Time)
		{
			char str[37];
			uuid_ToString(_Payload->m_UUID, str);
			printf("Removed: %s\n\r", str);
			LinkedList_RemoveItem(&_TransportLayer->m_Sented, _Payload);
			Payload_Dispose(_Payload);
		}

	}
	
}

void TransportLayer_Dispose(TransportLayer* _TransportLayer)
{
	LinkedList_Node* currentNode = _TransportLayer->m_Queued.m_Head;
	while(currentNode != NULL)
	{
		Payload* _Payload = (Payload*) currentNode->m_Item;
		currentNode = currentNode->m_Next;
		LinkedList_RemoveFirst(&_TransportLayer->m_Queued);
		Payload_Dispose(_Payload);
	}
	
	currentNode = _TransportLayer->m_Sented.m_Head;
	while(currentNode != NULL)
	{
		Payload* _Payload = (Payload*) currentNode->m_Item;
		currentNode = currentNode->m_Next;
		LinkedList_RemoveFirst(&_TransportLayer->m_Queued);
		Payload_Dispose(_Payload);
	}
	

	LinkedList_Dispose(&_TransportLayer->m_Sented);
	LinkedList_Dispose(&_TransportLayer->m_Queued);

	if(_TransportLayer->m_Allocated == True)
		Allocator_Free(_TransportLayer);
	else
		memset(_TransportLayer, 0, sizeof(TransportLayer));

}