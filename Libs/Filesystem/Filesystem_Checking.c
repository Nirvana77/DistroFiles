#include "Filesystem_Checking.h"

void Filesystem_Checking_ResetCheckingState(Filesystem_Checking* _Checking);

int Filesystem_Checking_InitializePtr(Filesystem_Server* _Server, Filesystem_Checking** _CheckingPtr)
{
	Filesystem_Checking* _Checking = (Filesystem_Checking*)Allocator_Malloc(sizeof(Filesystem_Checking));
	if(_Checking == NULL)
		return -1;
	
	int success = Filesystem_Checking_Initialize(_Checking, _Server);
	if(success != 0)
	{
		Allocator_Free(_Checking);
		return success;
	}
	
	_Checking->m_Allocated = True;
	
	*(_CheckingPtr) = _Checking;
	return 0;
}

int Filesystem_Checking_Initialize(Filesystem_Checking* _Checking, Filesystem_Server* _Server)
{
	_Checking->m_Allocated = False;
	_Checking->m_Server = _Server;

	int success = Payload_Initialize(&_Checking->m_Message, NULL);

	if(success != 0)
	{
		printf("Failed to initialize the payload!\n\r");
		printf("Error code: %i\n\r", success);
		return -2;
	}

	LinkedList_Initialize(&_Checking->m_List);
	
	return 0;
}


void Filesystem_Checking_RemoveCheck(Filesystem_Checking* _Checking, Filesystem_Checking_Check* _Check)
{
	_Check->m_IsUsed = False;
	_Check->m_Connection = NULL;

	LinkedList_Node* node = NULL;
	if(LinkedList_UnlinkItem(&_Checking->m_List, _Check, &node) == 0)
		LinkedList_LinkLast(&_Checking->m_List, node);
}

void Filesystem_Checking_Clear(Filesystem_Checking* _Checking)
{
	LinkedList_Node* currentNode = _Checking->m_List.m_Head;
	while (currentNode != NULL)
	{
		Filesystem_Checking_Check* check = (Filesystem_Checking_Check*)currentNode->m_Item;

		if(check->m_IsUsed == False)
			return;
		
		check->m_IsUsed = False;
		check->m_Connection = NULL;
		currentNode = currentNode->m_Next;
	}
	
	_Checking->m_Type = Filesystem_Checking_Type_None;
}

int Filesystem_Checking_SpawnWriteCheck(Filesystem_Checking* _Checking, Payload_Address* _Address, Filesystem_Checking_Check** _CheckPtr)
{
	if(_CheckPtr == NULL)
		return -1;

	LinkedList_Node* currentNode = _Checking->m_List.m_Head;
	while (currentNode != NULL)
	{
		Filesystem_Checking_Check* check = (Filesystem_Checking_Check*)currentNode->m_Item;

		if(check->m_IsUsed == False)
		{
			check->m_IsUsed = True;
			check->m_Connection = NULL;
			LinkedList_UnlinkNode(&_Checking->m_List, currentNode);
			LinkedList_LinkFirst(&_Checking->m_List, currentNode);
			*(_CheckPtr) = check;
			return 0;
		}
		else if(Payload_ComperAddresses(&check->m_Connection->m_Addrass, _Address) == True)
		{
			*(_CheckPtr) = check;
			return 1;
		}
		currentNode = currentNode->m_Next;
	}
	
	Filesystem_Checking_Check* check = (Filesystem_Checking_Check*) Allocator_Malloc(sizeof(Filesystem_Checking_Check));

	check->m_Connection = NULL;
	check->m_IsUsed = True;

	LinkedList_AddFirst(&_Checking->m_List, check);	
	*(_CheckPtr) = check;
	return 0;
}

int Filesystem_Checking_WorkOnPayload(Filesystem_Checking* _Checking, Filesystem_Checking_Type _Type, Payload* _Message)
{
	if(_Checking->m_Type != Filesystem_Checking_Type_None && (_Checking->m_Type != _Type || _Checking->m_Server->m_State == Filesystem_Server_State_Connecting))
		return 1;

	Filesystem_Checking_ResetCheckingState(_Checking);

	switch (_Type)
	{
		case Filesystem_Checking_Type_Write:
		{
			Bool isFile = True;
			Buffer_ReadUInt8(&_Message->m_Data, (UInt8*)&isFile);

			UInt16 size = 0;
			Buffer_ReadUInt16(&_Message->m_Data, &size);

			unsigned char path[size + 1];
			Buffer_ReadBuffer(&_Message->m_Data, path, size);
			path[size] = 0;
			String fullPath;

			String_Initialize(&fullPath, 64);
			String_Set(&fullPath, _Checking->m_Server->m_FilesytemPath.m_Ptr);

			if(String_EndsWith(&fullPath, "/") == False)
				String_Append(&fullPath, "/", 1);

			String_Append(&fullPath, (const char*)path, size);

			unsigned char hash[16] = "";
			unsigned char serverHash[16] = "";
			Buffer_ReadBuffer(&_Message->m_Data, hash, 16);
			Bool exist = False;

			if(isFile == True)
			{
				exist = File_Exist(fullPath.m_Ptr);
				if(exist == True)
					File_GetHash(fullPath.m_Ptr, serverHash);
			}
			else
			{
				exist = Folder_Exist(fullPath.m_Ptr);
				if(exist == True)
					Folder_Hash(fullPath.m_Ptr, serverHash);

			}

			if(exist == True)
			{
				Bool isSame = Filesystem_Server_HashCheck(serverHash, hash);

				Payload* msg = NULL;
				if(TransportLayer_CreateMessage(&_Checking->m_Server->m_TransportLayer, Payload_Type_Respons, 1 + 1 + (isSame == False ? 2 + size + 16 : 0), SEC, &msg) == 0)
				{
					Buffer_WriteUInt8(&msg->m_Data, (UInt8)_Type);
					if(isSame == True)
					{
						Buffer_WriteUInt8(&msg->m_Data, 0);
					}
					else
					{
						Buffer_WriteUInt8(&msg->m_Data, 1);
						Buffer_WriteUInt16(&msg->m_Data, size);
						Buffer_WriteBuffer(&msg->m_Data, path, size);
						Buffer_WriteBuffer(&msg->m_Data, serverHash, 16);
					}

					Payload_FilAddress(&msg->m_Des, &_Message->m_Src);
					Payload_SetMessageType(msg, Payload_Message_Type_String, "CheckAck", strlen("CheckAck"));
				}
			}
			else
			{
				Payload* msg = NULL;
				if(TransportLayer_CreateMessage(&_Checking->m_Server->m_TransportLayer, Payload_Type_Respons, 1 + 1, SEC, &msg) == 0)
				{
					Buffer_WriteUInt8(&msg->m_Data, (UInt8)_Type);
					Buffer_WriteUInt8(&msg->m_Data, 2);

					Payload_FilAddress(&msg->m_Des, &_Message->m_Src);
					Payload_SetMessageType(msg, Payload_Message_Type_String, "CheckAck", strlen("CheckAck"));
				}
			}
			
			String_Dispose(&fullPath);
		} break;

		case Filesystem_Checking_Type_Delete:
		{
			Bool isFile = True;
			Buffer_ReadUInt8(&_Message->m_Data, (UInt8*)&isFile);

			UInt16 size = 0;
			Buffer_ReadUInt16(&_Message->m_Data, &size);

			unsigned char path[size + 1];
			Buffer_ReadBuffer(&_Message->m_Data, path, size);
			path[size] = 0;
			String fullPath;

			String_Initialize(&fullPath, 64);
			String_Set(&fullPath, _Checking->m_Server->m_FilesytemPath.m_Ptr);

			if(String_EndsWith(&fullPath, "/") == False)
				String_Append(&fullPath, "/", 1);

			String_Append(&fullPath, (const char*)path, size);

			Bool exist = False;

			if(isFile == True)
				exist = File_Exist(fullPath.m_Ptr);
			
			else
				exist = Folder_Exist(fullPath.m_Ptr);

			if(exist == False)
			{
				int index = String_LastIndexOf(&fullPath, "/");
				String_SubString(&fullPath, index, fullPath.m_Length);

				unsigned char bufferHash[16] = "";
				unsigned char hash[16] = "";
				Buffer_ReadBuffer(&_Message->m_Data, bufferHash, 16);
				Folder_Hash(fullPath.m_Ptr, hash);

				Bool isSame = Filesystem_Server_HashCheck(bufferHash, hash);

				Payload* msg = NULL;
				if(TransportLayer_CreateMessage(&_Checking->m_Server->m_TransportLayer, Payload_Type_Respons, 1 + 1 + (isSame == False ? 2 + size + 16 : 0), SEC, &msg) == 0)
				{
					Buffer_WriteUInt8(&msg->m_Data, (UInt8)_Type);
					if(isSame == True)
					{
						Buffer_WriteUInt8(&msg->m_Data, 0);
					}
					else
					{
						Buffer_WriteUInt8(&msg->m_Data, 1);
						Buffer_WriteUInt16(&msg->m_Data, size);
						Buffer_WriteBuffer(&msg->m_Data, path, size);
						Buffer_WriteBuffer(&msg->m_Data, hash, 16);
					}

					Payload_FilAddress(&msg->m_Des, &_Message->m_Src);
					Payload_SetMessageType(msg, Payload_Message_Type_String, "CheckAck", strlen("CheckAck"));
				}

			}
			else
			{
				Payload* msg = NULL;
				if(TransportLayer_CreateMessage(&_Checking->m_Server->m_TransportLayer, Payload_Type_Respons, 1 + 1, SEC, &msg) == 0)
				{
					Buffer_WriteUInt8(&msg->m_Data, (UInt8)_Type);
					Buffer_WriteUInt8(&msg->m_Data, 2);

					Payload_FilAddress(&msg->m_Des, &_Message->m_Src);
					Payload_SetMessageType(msg, Payload_Message_Type_String, "CheckAck", strlen("CheckAck"));
				}
			}
			
			String_Dispose(&fullPath);
		} break;

		case Filesystem_Checking_Type_Syncing:
		{
			printf("Checking_Type is Filesystem_Checking_Type_Syncing!\r\n");

		} break;

		case Filesystem_Checking_Type_None:
		{
			printf("Checking_Type is Filesystem_Checking_Type_None!\r\n");
			Payload* msg = NULL;
			if(TransportLayer_CreateMessage(&_Checking->m_Server->m_TransportLayer, Payload_Type_Respons, 1 + 1, SEC, &msg) == 0)
			{
				Buffer_WriteUInt8(&msg->m_Data, (UInt8)_Type);
				Buffer_WriteUInt8(&msg->m_Data, 2);

				Payload_FilAddress(&msg->m_Des, &_Message->m_Src);
				Payload_SetMessageType(msg, Payload_Message_Type_String, "CheckAck", strlen("CheckAck"));
			}
			
			return -1;
		} break;
	}

	return 0;
}

void Filesystem_Checking_ResetCheckingState(Filesystem_Checking* _Checking)
{
	LinkedList_Node* currentNode = _Checking->m_List.m_Head;
	while (currentNode != NULL)
	{
		Filesystem_Checking_Check* check = (Filesystem_Checking_Check*) currentNode->m_Item;
		if(check->m_IsUsed == False)
			break;

		check->m_IsUsed = False;
		currentNode = currentNode->m_Next;
	}
}

Bool Filesystem_Checking_CanUseConnection(Filesystem_Checking* _Checking, Filesystem_Connection* _Connection)
{
	LinkedList_Node* node = _Checking->m_List.m_Head;
	while (node != NULL)
	{
		Filesystem_Checking_Check* check = (Filesystem_Checking_Check*) node->m_Item;

		if(check->m_IsUsed == False)
		{
			return True;
		}
		else if(_Connection == check->m_Connection)
		{
			if(check->m_IsOk != Filesystem_Checking_Check_Satus_OK)
				return False;
			
			return True;
		}
		else
		{
			node = node->m_Next;
		}
	}

	return True;
}

void Filesystem_Checking_Work(UInt64 _MSTime, Filesystem_Checking* _Checking)
{

	int size = 0;
	int oks = 0;
	int notSync = 0;
	LinkedList_Node* currentNode = _Checking->m_List.m_Head;
	while (currentNode != NULL)
	{
		Filesystem_Checking_Check* check = (Filesystem_Checking_Check*) currentNode->m_Item;

		if(check->m_IsUsed == False)
			break;

		if(check->m_IsOk == 0)
			oks++;

		else if(check->m_IsOk == 2)
			notSync++;

		size++;
		currentNode = currentNode->m_Next;
	}
	
	if(size != _Checking->m_Server->m_Connections.m_Size - 1)
		return;

	if(size - notSync != 0)
	{
		int error = (int)((double)(1 - oks / (size - notSync)) * 100);
		if(error >= Filesystem_Checking_CheckError)
		{
			_Checking->m_Type = Filesystem_Checking_Type_None;
			_Checking->m_Server->m_State = Filesystem_Server_State_ReSync;
			return;
		}
	}

	_Checking->m_Server->m_State = Filesystem_Server_State_Synced;

	currentNode = _Checking->m_List.m_Head;
	while (currentNode != NULL)
	{
		Filesystem_Checking_Check* check = (Filesystem_Checking_Check*) currentNode->m_Item;

		if(check->m_IsUsed == False)
			break;

		if(check->m_IsOk == Filesystem_Checking_Check_Satus_DontHave)
		{
			Payload* message = NULL;
			if(TransportLayer_CreateMessage(&_Checking->m_Server->m_TransportLayer, Payload_Type_Safe, _Checking->m_Message.m_Size, SEC, &message) == 0)
			{
				Payload_FilAddress(&_Checking->m_Message.m_Des, &check->m_Connection->m_Addrass);
				Payload_Copy(message, &_Checking->m_Message);
			}
		}

		size++;
		currentNode = currentNode->m_Next;
	}

	Filesystem_Checking_Clear(_Checking);
			

}

void Filesystem_Checking_Dispose(Filesystem_Checking* _Checking)
{
	LinkedList_Node* currentNode = _Checking->m_List.m_Head;
	while(currentNode != NULL)
	{
		Filesystem_Checking_Check* _Check = (Filesystem_Checking_Check*)currentNode->m_Item;
		currentNode = currentNode->m_Next;

		_Check->m_Connection = NULL;
		Allocator_Free(_Check);
		LinkedList_RemoveFirst(&_Checking->m_List);
	}

	Payload_Dispose(&_Checking->m_Message);

	if(_Checking->m_Allocated == True)
		Allocator_Free(_Checking);
	else
		memset(_Checking, 0, sizeof(Filesystem_Checking));

}