#ifndef String_h__
#define String_h__

struct T_String;
typedef struct T_String String;

#include "Allocator.h"

struct T_String
{
	Bool m_Allocated;
	int m_BufferSize;

	char* m_Ptr;
	int m_Length;
	int m_Size;

};

int String_InitializePtr(int _BufferSize, String** _StrPtr);
int String_Initialize(String* _Str, int _BufferSize);
void String_Dispose(String* _Str);


#endif // String_h__