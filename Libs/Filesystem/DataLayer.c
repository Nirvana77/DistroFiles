#include "DataLayer.h"

int DataLayer_InitializePtr(int (*_OnConnect)(void* _Context), int (*_OnRead)(void* _Context, Buffer* _Buffer, int _Size), int (*_OnWrite)(void* _Context, Buffer* _Buffer, int _Size), int (*_OnDisconnect)(void* _Context), void* _DataContext, UInt64 _Timeout, DataLayer** _DataLayerPtr)
{
    DataLayer* _DataLayer = (DataLayer*)Allocator_Malloc(sizeof(DataLayer));
    if(_DataLayer == NULL)
        return -1;
    
    int success = DataLayer_Initialize(_DataLayer, _OnConnect, _OnRead, _OnWrite, _OnDisconnect, _DataContext, _Timeout);
    if(success != 0)
    {
        Allocator_Free(_DataLayer);
        return success;
    }
    
    _DataLayer->m_Allocated = True;
    
    *(_DataLayerPtr) = _DataLayer;
    return 0;
}

int DataLayer_Initialize(DataLayer* _DataLayer, int (*_OnConnect)(void* _Context), int (*_OnRead)(void* _Context, Buffer* _Buffer, int _Size), int (*_OnWrite)(void* _Context, Buffer* _Buffer, int _Size), int (*_OnDisconnect)(void* _Context), void* _DataContext, UInt64 _Timeout)
{
    _DataLayer->m_Allocated = False;
    _DataLayer->m_DataContext = _Context;
    _DataLayer->m_OnConnect = _OnConnect;
    _DataLayer->m_OnRead = _OnRead;
    _DataLayer->m_OnWrite = _OnWrite;
    _DataLayer->m_OnDisconnect = _OnDisconnect;

    _DataLayer->m_Timeout = _Timeout;
    _DataLayer->m_NextHeartbeat = 0;


    
    return 0;
}

void DataLayer_Work(UInt64 _MSTime, DataLayer* _)
{
    if()
}


void DataLayer_Dispose(DataLayer* _DataLayer)
{
    

    if(_DataLayer->m_Allocated == True)
        Allocator_Free(_DataLayer);
    else
        memset(_DataLayer, 0, sizeof(DataLayer));

}