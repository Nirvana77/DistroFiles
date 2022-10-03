#ifndef Allocator_h__
#define Allocator_h__

typedef enum {
	Allocator_Bordercheck_OK = 0,
	Allocator_Bordercheck_FAILBEGIN = 1,
	Allocator_Bordercheck_FAILEND = 2,

} Allocator_Bordercheck;

#include "Memory.h"

void* Allocator_Malloc(unsigned int _Size);
int Allocator_Free(void* _Ptr);

#endif // Allocator_h__