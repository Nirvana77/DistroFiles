#include "Payload.h"

int Payload_InitializePtr(Payload** _PayloadPtr)
{
	Payload* _Payload = (Payload*)Allocator_Malloc(sizeof(Payload));
	if(_Payload == NULL)
		return -1;
	
	int success = Payload_Initialize(_Payload);
	if(success != 0)
	{
		Allocator_Free(_Payload);
		return success;
	}
	
	_Payload->m_Allocated = True;
	
	*(_PayloadPtr) = _Payload;
	return 0;
}

int Payload_Initialize(Payload* _Payload)
{
	_Payload->m_Allocated = False;
	
	return 0;
}




void Payload_Dispose(Payload* _Payload)
{
	

	if(_Payload->m_Allocated == True)
		Allocator_Free(_Payload);
	else
		memset(_Payload, 0, sizeof(Payload));

}