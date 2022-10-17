#include "NetworkLayer.h"

int NetworkLayer_InitializePtr(NetworkLayer** _NetworkLayerPtr)
{
    NetworkLayer* _NetworkLayer = (NetworkLayer*)Allocator_Malloc(sizeof(NetworkLayer));
    if(_NetworkLayer == NULL)
        return -1;
    
    int success = NetworkLayer_Initialize(_NetworkLayer);
    if(success != 0)
    {
        Allocator_Free(_NetworkLayer);
        return success;
    }
    
    _NetworkLayer->m_Allocated = True;
    
    *(_NetworkLayerPtr) = _NetworkLayer;
    return 0;
}

int NetworkLayer_Initialize(NetworkLayer* _NetworkLayer)
{
    _NetworkLayer->m_Allocated = False;
    
    return 0;
}

int NetworkLayer_SendPayload(void* _Context, Payload* _Paylode)
{
	NetworkLayer* _NetworkLayer = (NetworkLayer*) _Context;

	printf("NetworkLayer_SendPayload\n\r");

    UInt8 type = __UINT8_MAX__;
    
    Memory_ParseUInt8();

	_NetworkLayer->m_FuncOut.m_Send(_NetworkLayer->m_FuncOut.m_Context, _Paylode);


	return 0;
}

int NetworkLayer_ReveicePayload(void* _Context, Payload* _Paylode)
{
	NetworkLayer* _NetworkLayer = (NetworkLayer*) _Context;

	printf("NetworkLayer_ReveicePayload\n\r");
	
	Memory_ParseUInt16(_Paylode->m_Data.BUFFER, &_Paylode->m_Size);

	int reviced = _NetworkLayer->m_FuncOut.m_Receive(_NetworkLayer->m_FuncOut.m_Context, _Paylode);

	return 0;
}



void NetworkLayer_Dispose(NetworkLayer* _NetworkLayer)
{
    

    if(_NetworkLayer->m_Allocated == True)
        Allocator_Free(_NetworkLayer);
    else
        memset(_NetworkLayer, 0, sizeof(NetworkLayer));

}