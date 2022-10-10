#include "Datalayer.h"

int Datalayer_InitializePtr(Datalayer** _DatalayerPtr)
{
    Datalayer* _Datalayer = (Datalayer*)Allocator_Malloc(sizeof(Datalayer));
    if(_Datalayer == NULL)
        return -1;
    
    int success = Datalayer_Initialize(_Datalayer);
    if(success != 0)
    {
        Allocator_Free(_Datalayer);
        return success;
    }
    
    _Datalayer->m_Allocated = True;
    
    *(_DatalayerPtr) = _Datalayer;
    return 0;
}

int Datalayer_Initialize(Datalayer* _Datalayer)
{
    _Datalayer->m_Allocated = False;
    
    return 0;
}




void Datalayer_Dispose(Datalayer* _Datalayer)
{
    

    if(_Datalayer->m_Allocated == True)
        Allocator_Free(_Datalayer);
    else
        memset(_Datalayer, 0, sizeof(Datalayer));

}