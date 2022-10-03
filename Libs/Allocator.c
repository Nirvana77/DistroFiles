#include "Allocator.h"

#if defined ALLOCATOR_DEBUG_BORDERCHECK
	void Allocator_Print(void* _Ptr);
#endif

#ifdef ALLOCATOR_DEBUG
	void Allocator_Open(const char* _Path)
	{
		strcpy(g_Allocator.m_Path, _Path);
		File_Remove(g_Allocator.m_Path);
		File_Open(g_Allocator.m_Path, "a", &g_Allocator.m_F);
	}

	void Allocator_WriteEvent(Allocator_Event _Event, void* _Pointer, unsigned int _Size, const char* _FileString, unsigned int _LineNumber, const char* _FunctionString)
	{
		char str[1024];

		sprintf(str, "%i,%u,%s,%u,%s,%p\n",_Event, _Size, _FileString, _LineNumber, _FunctionString, _Pointer);

		File_WriteAll(g_Allocator.m_F, (unsigned char*)str, strlen(str));

	}

	void Allocator_Close()
	{
		File_SetPosition(g_Allocator.m_F, 0);
		int size = File_GetSize(g_Allocator.m_F);
		unsigned char data[size];
		memset(data, 0, size);

		//TODO Make read function
		
		File_Close(g_Allocator.m_F);
	}

	unsigned char* _Allocator_Malloc(unsigned int _Size, const char* _FileString, unsigned int _LineNumber, const char* _FunctionString)
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
		
		Allocator_WriteEvent(ALLOCATOR_EVENT_MALLOC, ptr, orginalSize, _FileString, _LineNumber, _FunctionString);
		
		return ptr;
	}

	int _Allocator_Free(void* _Ptr, const char* _FileString, unsigned int _LineNumber, const char* _FunctionString)
	{
		unsigned char* ptr = (unsigned char*)_Ptr;
		
		#ifdef ALLOCATOR_DEBUG_BORDERCHECK
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

		#endif
		
		Allocator_WriteEvent(ALLOCATOR_EVENT_FREE, _Ptr, 0, _FileString, _LineNumber, _FunctionString);

		#ifdef ALLOCATOR_DEBUG_BORDERCHECK

			ptr = (unsigned char*)_Ptr;
			ptr -= ALLOCATOR_DEBUG_BORDERCHECK;
			ptr -= sizeof(u_int32_t);
		#endif

		free(ptr);
		return 0;
	}

#else

	unsigned char* Allocator_Malloc(unsigned int _Size)
	{
		return NULL;
	}

	int Allocator_Free(void* _Ptr)
	{

		return 0;
	}

#endif

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

