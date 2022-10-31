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
	while(_Str->m_Length + _Length > _Str->m_Size)
	{
		if(String_ExtendBuffer(_Str) != 0)
			return -2;
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

int String_Exchange(String* _Str, const char* _Exp, const char* _Value)
{

	for (int i = 0; i < _Str->m_Length; i++)
	{
		if(_Str->m_Ptr[i] == _Exp[0])
		{
			Bool willExchange = True;
			int j = 1;
			for (; j < strlen(_Exp); j++)
			{
				if(_Str->m_Ptr[i + j] != _Exp[j])
				{
					willExchange = False;
					break;
				}
			}

			if(willExchange == True)
			{
				if(strlen(_Exp) == strlen(_Value))
				{
					
					for (int k = 0; k < strlen(_Value); k++)
						_Str->m_Ptr[i + k] = _Value[k];
					
					i += j;
				}
				else if(strlen(_Value) == 0)
				{
					int k = i;
					for (; k + j < _Str->m_Length; k++)
					{
						_Str->m_Ptr[k] = _Str->m_Ptr[k + j];
						if((char)_Str->m_Ptr[k] == 0)
							break;
						
					}
					memset(&_Str->m_Ptr[k], 0, _Str->m_Length - k);
					_Str->m_Length -= strlen(_Exp);
				}
				else
				{
					for (j = 0; j < strlen(_Value); j++)
					{
						printf("ERROR not supported\n\r");
						printf("%s and %s needs to be the same length\n\r", _Exp, _Value);
						printf("or %s needs 0 in length\n\r", _Value);
						return -1;
					}
					
				}
			}
			
		}
	}
	
	return 0;
}

int String_ReadFromFile(String* _Str, const char* _Path)
{
	FILE* f = NULL;
	if(File_Open(_Path, File_Mode_Read, &f) != 0)
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
	if(File_Open(_Path, File_Mode_WriteBinary, &f) != 0)
		return -1;

	int success = File_WriteAll(f, (unsigned char*)_Str->m_Ptr, _Str->m_Length);
	if(success <= 0)
	{
		printf("File write error\n\r");
		printf("Error code: %i\n\r", success);
		return -2;
	}

	File_Close(f);
	return success;
}

Bool String_EndsWith(String* _Str, const char* _Exp)
{
	int length = strlen(_Exp);

	if(_Str->m_Length - length < 0)
		return False;

	int j = 0;
	for (int i = _Str->m_Length - length; i < _Str->m_Length; i++)
	{
		if(_Str->m_Ptr[i] != _Exp[j++])
			return False;
	}
	
	return True;
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