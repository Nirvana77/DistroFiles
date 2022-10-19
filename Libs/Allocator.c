#include "Allocator.h"

#if defined ALLOCATOR_DEBUG_BORDERCHECK
	void Allocator_Print(void* _Ptr);
#endif

#ifdef ALLOCATOR_DEBUG
	int Allocator_CreateDatapoint(Allocator_Event _Event, void* _Pointer, unsigned int _Size, const char* _FileString, unsigned int _LineNumber, const char* _FunctionString)
	{
		if(_Event == ALLOCATOR_EVENT_FREE)
		{
			List_Node* currentNode = g_Allocator.m_Mallocs.m_Head;
			while (currentNode != NULL)
			{
				
				if(currentNode->m_Data.m_Ptr == _Pointer)
				{
					if(g_Allocator.m_Mallocs.m_Size == 1)
					{
						g_Allocator.m_Mallocs.m_Head = NULL;
						g_Allocator.m_Mallocs.m_Tail = NULL;
					}
					else if(currentNode == g_Allocator.m_Mallocs.m_Head)
					{
						currentNode->m_Next->m_Privios = NULL;
						g_Allocator.m_Mallocs.m_Head = currentNode->m_Next;
					}
					else if(currentNode == g_Allocator.m_Mallocs.m_Tail)
					{
						currentNode->m_Privios->m_Next = NULL;
						g_Allocator.m_Mallocs.m_Tail = currentNode->m_Privios;
					}
					else
					{
						currentNode->m_Privios->m_Next = currentNode->m_Next;
						currentNode->m_Next->m_Privios = currentNode->m_Privios;
					}
					
					g_Allocator.m_Mallocs.m_Size--;
					free(currentNode);
					return 0;
				}
				currentNode = currentNode->m_Next;
			}

			return -2;
		}

		List_Node* node = (List_Node*)malloc(sizeof(List_Node));

		if(node == NULL)
			return -1;

		strncpy((char*)node->m_Data.m_FileString, _FileString, sizeof(node->m_Data.m_FileString));
		strncpy((char*)node->m_Data.m_FunctionString, _FunctionString, sizeof(node->m_Data.m_FunctionString));
		
		node->m_Data.m_LineNumber = _LineNumber;
		node->m_Data.m_Size = _Size;
		node->m_Data.m_Ptr = _Pointer;

		if(g_Allocator.m_Mallocs.m_Size == 0)
		{
			g_Allocator.m_Mallocs.m_Head = node;
			g_Allocator.m_Mallocs.m_Tail = node;
			g_Allocator.m_Mallocs.m_Size++;
			return 0;
		}
		
		g_Allocator.m_Mallocs.m_Tail->m_Next = node;
		node->m_Privios = g_Allocator.m_Mallocs.m_Tail;
		g_Allocator.m_Mallocs.m_Tail = node;
		g_Allocator.m_Mallocs.m_Size++;

		return 0;
	}

	void Allocator_Open(const char* _Path)
	{
		strcpy(g_Allocator.m_Path, _Path);
		File_Remove(g_Allocator.m_Path);
		File_Open(g_Allocator.m_Path, "ab+", &g_Allocator.m_F);

		g_Allocator.m_Mallocs.m_Head = NULL;
		g_Allocator.m_Mallocs.m_Tail = NULL;
		g_Allocator.m_Mallocs.m_Size = 0;
	}

	void Allocator_WriteEvent(Allocator_Event _Event, void* _Pointer, unsigned int _Size, const char* _FileString, unsigned int _LineNumber, const char* _FunctionString)
	{
		char str[1024];

		sprintf(str, "%i,%u,%s,%u,%s,%p\n",_Event, _Size, _FileString, _LineNumber, _FunctionString, _Pointer);

		File_WriteAll(g_Allocator.m_F, (unsigned char*)str, strlen(str));
		Allocator_CreateDatapoint(_Event, _Pointer, _Size, _FileString, _LineNumber, _FunctionString);
	}

	void Allocator_Close()
	{
		File_SetPosition(g_Allocator.m_F, 0);/* 
		char * line = NULL;
		size_t len = 0;
    	ssize_t read;
		Allocator_Data node->m_Data[16];
		int i = 0;

		while ((read = getline(&line, &len, g_Allocator.m_F)) != -1) {
			printf("Retrieved line of length %zu:\n", read);
			printf("%s", line);
			node->m_Data[i]

			i++;

			if(i == 16)
			{
				
			}
		} */

		if(g_Allocator.m_Mallocs.m_Size != 0)
		{
			List_Node* currentNode = g_Allocator.m_Mallocs.m_Head;
			while (currentNode != NULL)
			{
				List_Node* node = currentNode;
				currentNode = currentNode->m_Next;
				printf("%u,%s,%u,%s,%p\n",node->m_Data.m_Size, node->m_Data.m_FileString, node->m_Data.m_LineNumber, node->m_Data.m_FunctionString, node->m_Data.m_Ptr);
				
				free(node);
			}
		}
		
		File_Close(g_Allocator.m_F);
	}

	unsigned char* _Allocator_Malloc(unsigned int _Size, const char* _FileString, unsigned int _LineNumber, const char* _FunctionString)
	{
		UInt32 orginalSize = (UInt32)_Size;

		#ifdef ALLOCATOR_DEBUG_BORDERCHECK
			_Size += sizeof(UInt32) + (ALLOCATOR_DEBUG_BORDERCHECK * 2);
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
				ptr -= sizeof(UInt32);
				UInt32 orginalSize = 0;
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
			ptr -= sizeof(UInt32);
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
		ptr -= sizeof(UInt32);
		UInt32 orginalSize = 0;
		ptr += Memory_ParseUInt32(ptr, &orginalSize);

		printf("Malloced Data:\r\n");
		int length = orginalSize + sizeof(UInt32) + ALLOCATOR_DEBUG_BORDERCHECK * 2;
		for (int i = 0; i < length; i++)
			printf("%x%s",ptr[i], i + 1 < length ? ", " : "");

		printf("\n\r");
	}
#endif

