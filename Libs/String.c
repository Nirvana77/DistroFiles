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
	_Str->m_Length = 0;
	
	_Str->m_Size = _Str->m_BufferSize + 1;//!Adding the zero bit

	_Str->m_Ptr = (char*) Allocator_Malloc(sizeof(char) * (_Str->m_Size)); 

	return 0;
}

int String_ExtendBuffer(String* _Str)
{
	int newSize = _Str->m_Size + _Str->m_BufferSize;
	char* newPtr = (char*) Allocator_Malloc(newSize*sizeof(char));
	
	if(newPtr == NULL)
		return -1;
	memcpy(newPtr, _Str->m_Ptr, _Str->m_Size);

	Allocator_Free(_Str->m_Ptr);

	_Str->m_Ptr = newPtr;
	_Str->m_Size = newSize;
	
	return 0;
}

int String_Set(String* _Str, const char* _String)
{
	if(strlen(_String) + 1 > _Str->m_Size) //!Zero bite
	{
		int newSize = (int)(strlen(_String)/_Str->m_BufferSize) * _Str->m_BufferSize + _Str->m_BufferSize + 1; //!Zero bite
		char* newPtr = (char*) Allocator_Malloc(newSize*sizeof(char));

		if(newPtr == NULL)
			return -1;
		
		Allocator_Free(_Str->m_Ptr);
		_Str->m_Ptr = newPtr;
		_Str->m_Size = newSize;

	}

	memcpy(_Str->m_Ptr, _String, strlen(_String) + 1);
	_Str->m_Length = strlen(_String);

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

int String_Sprintf(String* _Str, const char* _String, ...)
{
	va_list param;

	va_start(param, _String);
	int printLength;
	int maxLength = _Str->m_Size - _Str->m_Length - 1;

	#ifdef __linux__
		printLength = vsnprintf((char*)&_Str->m_Ptr[_Str->m_Length], maxLength, _String, param);
	#else
		printf("You system is not supported for sprintf\n\r");
	#endif

	va_end(param);

	if(printLength < 0)
		return -1;

	if(printLength >= maxLength)
	{
		while(printLength >= maxLength)
		{
			if(String_ExtendBuffer(_Str) != 0)
				return -2;
				
			maxLength = _Str->m_Size - _Str->m_Length - 1;
		}

		va_start(param, _String);

		#ifdef __linux__
			printLength = vsnprintf((char*)&_Str->m_Ptr[_Str->m_Length], maxLength, _String, param);
		#else
			printf("You system is not supported for sprintf\n\r");
		#endif

		va_end(param);
	}

	_Str->m_Length += printLength;
	_Str->m_Ptr[_Str->m_Length] = 0;

	return 0;
}

int String_ReadFromFile(String* _Str, const char* _Path)
{
	FILE* f = NULL;
	if(File_Open(_Path, "r", &f) != 0)
		return -1;

	int fileSize = File_GetSize(f);
	if(_Str->m_Size < fileSize)
	{
		char* newPtr = (char*) Allocator_Malloc(sizeof(char) * ((int)(fileSize/_Str->m_BufferSize) + _Str->m_BufferSize));

		if(newPtr == NULL)
			return -2;

		Allocator_Free(_Str->m_Ptr);
		_Str->m_Ptr = newPtr;
	}

	File_ReadAll(f, (unsigned char*)_Str->m_Ptr, _Str->m_Size);

	File_Close(f);
	return 0;
}

int String_SaveToFile(String* _Str, const char* _Path)
{
	FILE* f = NULL;
	if(File_Open(_Path, "wb", &f) != 0)
		return -1;

	File_WriteAll(f, (unsigned char*)_Str->m_Ptr, _Str->m_Length);

	File_Close(f);
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