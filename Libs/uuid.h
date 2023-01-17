#ifndef uuid_h__
#define uuid_h__

struct T_uuid;
typedef struct T_uuid uuid;

#include <uuid/uuid.h>
#include "Types.h"

#define UUID_DATA_SIZE 16
#define UUID_STRING_SIZE 33
#define UUID_FULLSTRING_SIZE 37

struct T_uuid
{
	Bool m_Allocated;
	UInt8 m_UUID[UUID_DATA_SIZE];
	unsigned char m_Str[UUID_STRING_SIZE];

};

int uuid_InitializePtr(uuid** _UUIDPtr);
int uuid_Initialize(uuid* _UUID);

int uuid_ToString(UInt8 _Data[UUID_DATA_SIZE], char _Buffer[UUID_FULLSTRING_SIZE]);
Bool uuid_Compere(UInt8 _A[UUID_DATA_SIZE], UInt8 _B[UUID_DATA_SIZE]);

static inline int uuid_Copy(UInt8 _Des[UUID_DATA_SIZE], UInt8 _Src[UUID_DATA_SIZE])
{
	return Memory_Copy(_Des, _Src, UUID_DATA_SIZE);
}

void uuid_Dispose(uuid* _UUID);

#endif // uuid_h__