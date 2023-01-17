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

int uuid_ToString(UInt8 _Data[UUID_DATA_SIZE], char _Buffer[UUID_FULLSTRING_SIZE])
{
	char* ptr = _Buffer;
	int i;
	for(i = 0; i < UUID_DATA_SIZE; i++)
		ptr += sprintf(ptr, "%02x%s", _Data[i], (i == 3 || i == 5 || i == 7 || i == 9) ? "-" : "");
	
	*(ptr++) = 0;
	return 0;
}

Bool uuid_Compere(UInt8 _A[UUID_DATA_SIZE], UInt8 _B[UUID_DATA_SIZE])
{
	for (int i = 0; i < UUID_DATA_SIZE; i++)
	{
		if(_A[i] != _B[i])
			return False;
	}
	
	return True;
}

void uuid_Dispose(uuid* _UUID)
{
	

	if(_UUID->m_Allocated == True)
		Allocator_Free(_UUID);
	else
		memset(_UUID, 0, sizeof(uuid));

}