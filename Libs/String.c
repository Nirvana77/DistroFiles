#include "String.h"

int String_ExtendBuffer(String* _Str);

int String_InitializePtr(int _BufferSize, String** _StrPtr)
{
	String* _Server = (String*)malloc(sizeof(String));
	if(_Server == NULL)
		return -1;
	
	int success = String_Initialize(_Server, _BufferSize);
	if(success != 0)
	{
		free(_Server);
		return success;
	}
	
	_Server->m_Allocated = True;
	
	*(_StrPtr) = _Server;
	return 0;
}

int String_Initialize(String* _Str, int _BufferSize)
{
	_Str->m_Allocated = False;
	_Str->m_BufferSize = _BufferSize;
	//Adding the zero bit
	_Str->m_Size = _Str->m_BufferSize + 1;

	_Str->m_Ptr = (char*) Allocator_Malloc(sizeof(char) * (_Str->m_Size)); 

	return 0;
}

int String_ExtendBuffer(String* _Str)
{
	int newSize = _Str->m_Size + _Str->m_BufferSize;
	char* newPtr = (char*) Allocator_Malloc(newSize*sizeof(char));
	
	if(newPtr == NULL)
		return -1;

	if(memcpy(newPtr, _Str->m_Ptr, _Str->m_Size) != 0)
	{
		Allocator_Free(newPtr);
		return -2;
	}

	Allocator_Free(_Str->m_Ptr);

	_Str->m_Ptr = newPtr;
	_Str->m_Size = newSize;
	
	return 0;
}

int String_Append(String* _Str, const char* _String, int _Length)
{
	if(_Str->m_Length + _Length > _Str->m_Size) {
		if(String_ExtendBuffer(_Str) != 0)
			return -1;

	}

	memcpy(&_Str->m_Ptr[_Str->m_Length], _String, _Length + 1);
	_Str->m_Length += _Length;

	return 0;
}

void String_Dispose(String* _Str)
{
	if(_Str->m_Ptr != NULL)
		Allocator_Free(_Str->m_Ptr);

	if(_Str->m_Allocated == True)
		Allocator_Free(_Str);
	else
		memset(_Str, 0, sizeof(String));

}