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
    
    return 0;
}



void TransportLayer_Dispose(TransportLayer* _TransportLayer)
{
    

    if(_TransportLayer->m_Allocated == True)
        Allocator_Free(_TransportLayer);
    else
        memset(_TransportLayer, 0, sizeof(TransportLayer));

}