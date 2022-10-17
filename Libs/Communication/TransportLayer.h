#ifndef TransportLayer_h__
#define TransportLayer_h__

struct T_TransportLayer;
typedef struct T_TransportLayer TransportLayer;

#include "Payload.h"
#include "../LinkedList.h"

struct T_TransportLayer
{
	Bool m_Allocated;

	LinkedList m_Queued;

	Payload_FuncOut m_FuncOut;
	LinkedList_Node* m_CurrentNode;
	
};

int TransportLayer_InitializePtr(TransportLayer** _TransportLayerPtr);
int TransportLayer_Initialize(TransportLayer* _TransportLayer);

int TransportLayer_CreateMessage(TransportLayer* _TransportLayer, Payload_Type _Type, int _Size, Payload** _PayloadPtr);
int TransportLayer_DestroyMessage(TransportLayer* _TransportLayer, Payload* _Payload);
int TransportLayer_RemoveMessage(TransportLayer* _TransportLayer, Payload* _Payload);
int TransportLayer_SendMessage(TransportLayer* _TransportLayer);


int TransportLayer_SendPayload(void* _Context, Payload* _Paylode);
int TransportLayer_ReveicePayload(void* _Context, Payload* _Replay);

void TransportLayer_Work(UInt64 _MSTime, TransportLayer* _TransportLayer);


void TransportLayer_Dispose(TransportLayer* _TransportLayer);
#endif // TransportLayer_h__