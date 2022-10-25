#ifndef NetworkLayer_h__
#define NetworkLayer_h__

struct T_NetworkLayer;
typedef struct T_NetworkLayer NetworkLayer;

#include "Payload.h"
#include "../Buffer.h"

struct T_NetworkLayer
{
	Bool m_Allocated;
	
	Payload_FuncOut m_FuncOut;

};

int NetworkLayer_InitializePtr(NetworkLayer** _NetworkLayerPtr);
int NetworkLayer_Initialize(NetworkLayer* _NetworkLayer);


int NetworkLayer_SendPayload(void* _Context, Payload** _PaylodePtr);
int NetworkLayer_ReveicePayload(void* _Context, Payload* _Message, Payload* _Replay);

void NetworkLayer_Dispose(NetworkLayer* _NetworkLayer);
#endif // NetworkLayer_h__