#include "Allocator.h"

int Memory_UInt32ToBuffer(u_int32_t* _Src, unsigned char* _Pointer);
int Memory_ParseUInt32(const void* _Pointer, u_int32_t* _Dest);

#if defined ALLOCATOR_DEBUG_BORDERCHECK
	void Allocator_Print(void* _Ptr);
#endif

void* Allocator_Malloc(unsigned int _Size)
{
	u_int32_t orginalSize = (u_int32_t)_Size;

	#ifdef ALLOCATOR_DEBUG_BORDERCHECK
		_Size += sizeof(u_int32_t) + (ALLOCATOR_DEBUG_BORDERCHECK * 2);
	#endif

	unsigned char* ptr = (unsigned char*)malloc(_Size);

	#if defined ALLOCATOR_DEBUG_BORDERCHECK
		if(ptr != NULL)
		{
			unsigned char* endPtr = ptr;
			endPtr += _Size;
			ptr += Memory_UInt32ToBuffer(&orginalSize, ptr);
			unsigned int i;
			for(i = 0; i < ALLOCATOR_DEBUG_BORDERCHECK; i++)
			{
				*(ptr++) = i;
				*(endPtr--) = i;
			}
		}
	#endif

	char data[126];
	sprintf(data, "Allocated(%i): %p\n", _Size, ptr);
	FILE* f = NULL;
	File_Open("AllocatorDebug.txt", "a", &f);
	File_Append(f, data, strlen(data));
	File_Close(f);

	return ptr;
}

int Memory_UInt32ToBuffer(u_int32_t* _Src, unsigned char* _Pointer)
{
	_Pointer[0] = (unsigned char)((*_Src >> 24) & 0xFF);
	_Pointer[1] = (unsigned char)((*_Src >> 16) & 0xFF);
	_Pointer[2] = (unsigned char)((*_Src >> 8) & 0xFF);
	_Pointer[3] = (unsigned char)((*_Src & 0xFF));
	return 4;
}

int Memory_ParseUInt32(const void* _Pointer, u_int32_t* _Dest)
{
	*_Dest = 0;
	*_Dest = (u_int32_t)((unsigned char*)_Pointer)[0]; *_Dest <<= 8;
	*_Dest |= (u_int32_t)((unsigned char*)_Pointer)[1]; *_Dest <<= 8;
	*_Dest |= (u_int32_t)((unsigned char*)_Pointer)[2]; *_Dest <<= 8;
	*_Dest |= (u_int32_t)((unsigned char*)_Pointer)[3];
	return 4;
}

int Allocator_Free(void* _Ptr)
{
	unsigned char* ptr = (unsigned char*)_Ptr;
	#if defined ALLOCATOR_DEBUG_BORDERCHECK
		if(ptr != NULL)
		{
			Allocator_Bordercheck status = Allocator_Bordercheck_OK;
			ptr -= ALLOCATOR_DEBUG_BORDERCHECK;
			ptr -= sizeof(u_int32_t);
			u_int32_t orginalSize = 0;
			ptr += Memory_ParseUInt32(ptr, &orginalSize);
			unsigned char* endPtr = ptr + ALLOCATOR_DEBUG_BORDERCHECK + orginalSize + ALLOCATOR_DEBUG_BORDERCHECK - 1;
			
			unsigned int i;
			for(i = 0; i < ALLOCATOR_DEBUG_BORDERCHECK; i++)
			{
				if(*(ptr++) != i)
				{
					fprintf(stderr, "Border Check Failed in the begining\n\r");
					status = Allocator_Bordercheck_FAILBEGIN;
					break;
				}
				else if(*(endPtr--) == i)
				{
					fprintf(stderr, "Border Check Failed in the ending\n\r");
					status = Allocator_Bordercheck_FAILEND;
					break;
				}
					
			}
			if(status != Allocator_Bordercheck_OK)
				Allocator_Print(_Ptr);
		}

		

		ptr = (unsigned char*)_Ptr;
		ptr -= ALLOCATOR_DEBUG_BORDERCHECK;
		ptr -= sizeof(u_int32_t);
	#endif

	
	char data[126];
	sprintf(data, "Freed(%p): %p\n", ptr, _Ptr);
	FILE* f = NULL;
	File_Open("AllocatorDebug.txt", "a", &f);
	File_Append(f, data, strlen(data));
	File_Close(f);


	free(ptr);
	return 0;
}

#if defined ALLOCATOR_DEBUG_BORDERCHECK
	void Allocator_Print(void* _Ptr)
	{
		unsigned char* ptr = (unsigned char*)_Ptr;

		ptr -= ALLOCATOR_DEBUG_BORDERCHECK;
		ptr -= sizeof(u_int32_t);
		u_int32_t orginalSize = 0;
		ptr += Memory_ParseUInt32(ptr, &orginalSize);

		int length = orginalSize + sizeof(u_int32_t) + ALLOCATOR_DEBUG_BORDERCHECK * 2;
		for (int i = 0; i < length; i++)
			printf("%x%s",ptr[i], i + 1 < length ? ", " : "");

		printf("\n\r");
	}
#endif