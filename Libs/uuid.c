#include "uuid.h"

int uuid_InitializePtr(uuid** _UUIDPtr)
{
	uuid* _UUID = (uuid*)Allocator_Malloc(sizeof(uuid));
	if(_UUID == NULL)
		return -1;
	
	int success = uuid_Initialize(_UUID);
	if(success != 0)
	{
		Allocator_Free(_UUID);
		return success;
	}
	
	_UUID->m_Allocated = True;
	
	*(_UUIDPtr) = _UUID;
	return 0;
}

int uuid_Initialize(uuid* _UUID)
{
	_UUID->m_Allocated = False;
	
	return 0;
}

int uuid_ToString(UInt8 _Data[UUID_DATA_SIZE], char _Buffer[37])
{
	char* ptr = _Buffer;
	int i;
	for(i = 0; i < UUID_DATA_SIZE; i++)
		ptr += sprintf(ptr, "%02x%s", _Data[i], (i == 3 || i == 5 || i == 7 || i == 9) ? "-" : "");
	
	*(ptr++) = 0;
	return 0;
}


void uuid_Dispose(uuid* _UUID)
{
	

	if(_UUID->m_Allocated == True)
		Allocator_Free(_UUID);
	else
		memset(_UUID, 0, sizeof(uuid));

}