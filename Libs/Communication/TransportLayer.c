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
	success = LinkedList_Initialize(&_TransportLayer->m_Sent);
	if(success != 0)
	{
		printf("Failed to initialize the Sented!\n\r");
		printf("Error code: %i\n\r", success);
		return -2;
	}
	success = LinkedList_Initialize(&_TransportLayer->m_Postponed);
	if(success != 0)
	{
		printf("Failed to initialize the Postponed!\n\r");
		printf("Error code: %i\n\r", success);
		return -2;
	}
	
	return 0;
}

int TransportLayer_CreateMessage(TransportLayer* _TransportLayer, Payload_Type _Type, int _Size, int _Timeout, Payload** _PayloadPtr)
{

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

	_Payload->m_State = Payload_State_Destroyed;
	EventHandler_EventCall(&_Payload->m_EventHandler, (int)_Payload->m_State, _Payload);

	Payload_Dispose(_Payload);

	return 0;
}

int TransportLayer_ResendMessage(TransportLayer* _TransportLayer, Payload* _Payload, Payload** _PayloadPtr)
{
	Payload* message = NULL;
	if(TransportLayer_CreateMessage(_TransportLayer, _Payload->m_Type, _Payload->m_Size, _Payload->m_Timeout, &message) == 0)
	{
		Payload_Copy(message, _Payload);
		if(_PayloadPtr != NULL)
			*(_PayloadPtr) = message;
			
		return 0;
	}
	return -1;
}

int TransportLayer_RemoveMessage(TransportLayer* _TransportLayer, Payload* _Payload)
{
	return LinkedList_RemoveItem(&_TransportLayer->m_Queued, _Payload);
}

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
		SystemMonotonicMS(&p->m_Time);

		LinkedList_Push(&_TransportLayer->m_Sent, p);
		
		p->m_State = Payload_State_Sending;
		EventHandler_EventCall(&p->m_EventHandler, (int)p->m_State, p);

		*(_PaylodePtr) = p;
		
		return 1;
 	}


	return 0;
}

int TransportLayer_ReveicePayload(void* _Context, Payload* _Message, Payload* _Replay)
{
	TransportLayer* _TransportLayer = (TransportLayer*) _Context;
	
	UInt8 UUID[UUID_DATA_SIZE];
	Buffer_ReadBuffer(&_Message->m_Data, UUID, UUID_DATA_SIZE);
	memcpy(_Message->m_UUID, UUID, UUID_DATA_SIZE);

	Payload_ReadMessage(&_Message->m_Message, &_Message->m_Data);

	Buffer_ReadUInt16(&_Message->m_Data, &_Message->m_Size);

	Payload_Print(_Message, "ReveicePayload");

	if(_Message->m_Type == Payload_Type_BroadcastRespons || _Message->m_Type == Payload_Type_Respons)
	{
		Bool hasMessage = False;
		LinkedList_Node* currentNode = _TransportLayer->m_Sent.m_Head;
		while(currentNode != NULL)
		{
			Payload* _Payload = (Payload*) currentNode->m_Item;
			currentNode = currentNode->m_Next;
			
			if(uuid_Compere(_Payload->m_UUID, _Message->m_UUID) == True)
			{
				EventHandler_EventCall(&_Payload->m_EventHandler, (int)Payload_State_Replay, _Message);
				hasMessage = True;
				SystemMonotonicMS(&_Payload->m_Time);
				_Payload->m_State = Payload_State_Replay;
				currentNode = NULL;
			}
		}
				
		if(hasMessage == False)
		{
			char str[UUID_FULLSTRING_SIZE];
			uuid_ToString(_Message->m_UUID, str);
			printf("Discarding %s\r\n", str);
			return 0;
		}
	}

	if(_TransportLayer->m_FuncOut.m_Receive != NULL)
	{
		Payload* replay;
		Payload_InitializePtr(_Message->m_UUID, &replay);
		unsigned char* readPtr = _Message->m_Data.m_ReadPtr;
		int revice = _TransportLayer->m_FuncOut.m_Receive(_TransportLayer->m_FuncOut.m_Context, _Message, replay);
		SystemMonotonicMS(&replay->m_Time);
		if(revice == 1)
		{
			if(_Message->m_Type == Payload_Type_UnSafe || _Message->m_Type == Payload_Type_Safe)
				_Message->m_Type = Payload_Type_Respons;
			else if(_Message->m_Type == Payload_Type_Broadcast)
				_Message->m_Type = Payload_Type_BroadcastRespons;
			
			Payload_FilAddress(&replay->m_Des, &_Message->m_Src);
			LinkedList_Push(&_TransportLayer->m_Queued, replay);
			return 0;
		}
		else if(revice == 2)
		{
			_Message->m_Data.m_BytesLeft += _Message->m_Data.m_ReadPtr - readPtr;
			_Message->m_Data.m_ReadPtr = readPtr;
			Payload_Copy(replay, _Message);
			replay->m_Time += SEC * 2;

			Payload_Print(replay, "Postponed");
			
			LinkedList_Push(&_TransportLayer->m_Postponed, replay);
			return 0;
		}
		Payload_Dispose(replay);
	}

	return 0;
}

void TransportLayer_Work(UInt64 _MSTime, TransportLayer* _TransportLayer)
{
	LinkedList_Node* currentNode = _TransportLayer->m_Sent.m_Head;
	while(currentNode != NULL)
	{
		Payload* _Payload = (Payload*) currentNode->m_Item;
		currentNode = currentNode->m_Next;
		
		if(_MSTime > _Payload->m_Timeout + _Payload->m_Time)
		{
			char str[UUID_FULLSTRING_SIZE];
			uuid_ToString(_Payload->m_UUID, str);
			printf("Removed: %s\n\r", str);
			
			if(_Payload->m_State == Payload_State_Resived)
				_Payload->m_State = Payload_State_Destroyed;
			else
				_Payload->m_State = Payload_State_Timeout;
			
			EventHandler_EventCall(&_Payload->m_EventHandler, (int)_Payload->m_State, _Payload);
			
			LinkedList_RemoveItem(&_TransportLayer->m_Sent, _Payload);
			Payload_Dispose(_Payload);
		}

	}

	currentNode = _TransportLayer->m_Postponed.m_Head;
	while(currentNode != NULL)
	{
		LinkedList_Node* node = currentNode;
		Payload* _Payload = (Payload*) node->m_Item;
		currentNode = currentNode->m_Next;
		
		if(_MSTime > _Payload->m_Time)
		{
			char str[UUID_FULLSTRING_SIZE];
			uuid_ToString(_Payload->m_UUID, str);
			printf("Prossessing postponed messaged: %s\n\r", str);
			LinkedList_RemoveNode(&_TransportLayer->m_Postponed, node);
			TransportLayer_ReveicePayload(_TransportLayer, _Payload, NULL);
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
	
	currentNode = _TransportLayer->m_Sent.m_Head;
	while(currentNode != NULL)
	{
		Payload* _Payload = (Payload*) currentNode->m_Item;
		currentNode = currentNode->m_Next;
		LinkedList_RemoveFirst(&_TransportLayer->m_Sent);
		Payload_Dispose(_Payload);
	}

	currentNode = _TransportLayer->m_Postponed.m_Head;
	while(currentNode != NULL)
	{
		Payload* _Payload = (Payload*) currentNode->m_Item;
		currentNode = currentNode->m_Next;
		LinkedList_RemoveFirst(&_TransportLayer->m_Postponed);
		Payload_Dispose(_Payload);
	}
	

	LinkedList_Dispose(&_TransportLayer->m_Postponed);
	LinkedList_Dispose(&_TransportLayer->m_Sent);
	LinkedList_Dispose(&_TransportLayer->m_Queued);

	if(_TransportLayer->m_Allocated == True)
		Allocator_Free(_TransportLayer);
	else
		memset(_TransportLayer, 0, sizeof(TransportLayer));

}