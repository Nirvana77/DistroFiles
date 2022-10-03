#ifndef Allocator_h__
#define Allocator_h__

typedef enum {
	Allocator_Bordercheck_OK = 0,
	Allocator_Bordercheck_FAILBEGIN = 1,
	Allocator_Bordercheck_FAILEND = 2,

} Allocator_Bordercheck;

typedef enum
{
	ALLOCATOR_EVENT_MALLOC = 0,
	ALLOCATOR_EVENT_FREE = 1

} Allocator_Event;

struct T_Allocator;
typedef struct T_Allocator Allocator;

#include "Memory.h"

struct T_Allocator
{
	char m_Path[1024];
	FILE* m_F;

};


#ifdef ALLOCATOR_DEBUG
	#define Allocator_Malloc(_SIZE) _Allocator_Malloc(_SIZE, __FILE__ , __LINE__, __FUNCTION__)
	#define Allocator_Free(_ALLOCATEDMEMORY) _Allocator_Free(_ALLOCATEDMEMORY, __FILE__ , __LINE__, __FUNCTION__)

	unsigned char* _Allocator_Malloc(unsigned int _Size, const char* _FileString, unsigned int _LineNumber, const char* _FunctionString);
	int _Allocator_Free(void* _Ptr, const char* _FileString, unsigned int _LineNumber, const char* _FunctionString);

	Allocator g_Allocator;
	void Allocator_Open(const char* _Path);
	void Allocator_WriteEvent(Allocator_Event _Event, void* _Pointer, unsigned int _Size, const char* _FileString, unsigned int _LineNumber, const char* _FunctionString);
	
	void Allocator_Close();
#else

	unsigned char* Allocator_Malloc(unsigned int _Size);
	int Allocator_Free(void* _Ptr);

#endif


#endif // Allocator_h__