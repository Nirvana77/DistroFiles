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
	
	return 0;
}

int TransportLayer_CreateMessage(TransportLayer* _TransportLayer, Payload_Type _Type, int _Size, Payload** _PayloadPtr)
{
	if(_Size == 0)
		return -2;

	Payload* _Payload = NULL;

	if(Payload_InitializePtr(&_Payload) != 0)
		return -1;

	LinkedList_Push(&_TransportLayer->m_Queued, _Payload);
	
	_Payload->m_Size = _Size;

	SystemMonotonicMS(&_Payload->m_Time);

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

int TransportLayer_SendMessage(TransportLayer* _TransportLayer)
{
	if(_TransportLayer->m_Queued.m_Head == NULL)
		return 0;

	Payload* _Payload = (Payload*)LinkedList_RemoveFirst(&_TransportLayer->m_Queued);
	
	//TODO #16 Create a sented list for respons

	int success = _TransportLayer->m_FuncOut.m_Send(_TransportLayer->m_FuncOut.m_Context, _Payload);
	return success;
}

//TODO #17 Fix this to be a propper transport layer
int TransportLayer_SendPayload(void* _Context, Payload* _Paylode)
{
	TransportLayer* _TransportLayer = (TransportLayer*) _Context;

	printf("TransportLayer_SendPayload\n\r");

	_TransportLayer->m_FuncOut.m_Send(_TransportLayer->m_FuncOut.m_Context, _Paylode);


	return 0;
}

int TransportLayer_ReveicePayload(void* _Context, Payload* _Paylode)
{
	TransportLayer* _TransportLayer = (TransportLayer*) _Context;

	printf("TransportLayer_ReveicePayload\n\r");

	_TransportLayer->m_FuncOut.m_Receive(_TransportLayer->m_FuncOut.m_Context, _Paylode);

	return 0;
}


void TransportLayer_Work(UInt64 _MSTime, TransportLayer* _TransportLayer)
{
	if(_TransportLayer->m_CurrentNode != NULL)
	{
		Payload* _Payload = (Payload*) _TransportLayer->m_CurrentNode->m_Item;

		int success = _TransportLayer->m_FuncOut.m_Send(_TransportLayer->m_FuncOut.m_Context, _Payload);
		if(success < 0)
		{
			printf("TransportLayer_Work funcOut failed\n\r");
			printf("Error code: %i\n\r", success);
			return;
		}


		_TransportLayer->m_CurrentNode = _TransportLayer->m_CurrentNode->m_Next;
		LinkedList_RemoveItem(&_TransportLayer->m_Queued, _Payload);
		Payload_Dispose(_Payload);

		if(_TransportLayer->m_CurrentNode == NULL)
			_TransportLayer->m_CurrentNode = _TransportLayer->m_Queued.m_Head;
		
	}
	else if (_TransportLayer->m_Queued.m_Head != NULL)
	{
		_TransportLayer->m_CurrentNode = _TransportLayer->m_Queued.m_Head;
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
	

	LinkedList_Dispose(&_TransportLayer->m_Queued);

	if(_TransportLayer->m_Allocated == True)
		Allocator_Free(_TransportLayer);
	else
		memset(_TransportLayer, 0, sizeof(TransportLayer));

}