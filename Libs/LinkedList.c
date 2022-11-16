#include "LinkedList.h"

int LinkedList_InitializePtr(LinkedList** _ListPtr)
{
	LinkedList* _List = (LinkedList*)Allocator_Malloc(sizeof(LinkedList));
	if(_List == NULL)
		return -1;
	
	int success = LinkedList_Initialize(_List);
	if(success != 0)
	{
		Allocator_Free(_List);
		return success;
	}
	
	_List->m_Allocated = True;
	
	*(_ListPtr) = _List;
	return 0;
}

int LinkedList_Initialize(LinkedList* _List)
{
	_List->m_Allocated = False;
	_List->m_Head = NULL;
	_List->m_Tail = NULL;
	_List->m_Size = 0;
	
	return 0;
}

int LinkedList_LinkFirst(LinkedList* _List, LinkedList_Node* _Node)
{
	if(_List->m_Size == 0)
	{
		_List->m_Head = _Node;
		_List->m_Tail = _Node;
		_List->m_Size = 1;
		return 0;
	}

	_List->m_Head->m_Privios = _Node;
	_Node->m_Next = _List->m_Head;
	_List->m_Head = _Node;
	_List->m_Size++;

	return 0;
}

int LinkedList_LinkLast(LinkedList* _List, LinkedList_Node* _Node)
{

	if(_List->m_Size == 0)
	{
		_List->m_Head = _Node;
		_List->m_Tail = _Node;
		_List->m_Size = 1;
		return 0;
	}
	
	_List->m_Tail->m_Next = _Node;
	_Node->m_Privios = _List->m_Tail;
	_List->m_Tail = _Node;
	_List->m_Size++;

	return 0;
}

LinkedList_Node* LinkedList_UnlinkFirst(LinkedList* _List)
{
	LinkedList_Node* head = _List->m_Head;

	if(_List->m_Size == 0)
		return NULL;

	if(_List->m_Size == 1)
	{
		_List->m_Head = NULL;
		_List->m_Tail = NULL;

	}
	else
	{
		_List->m_Head = head->m_Next;
		_List->m_Head->m_Privios = NULL;

	}

	_List->m_Size--;
	
	head->m_Next = NULL;
	head->m_Privios = NULL;

	return head;
}

int LinkedList_UnlinkAt(LinkedList* _List, int _Index, LinkedList_Node** _NodePtr)
{
	if(_Index < 0 || _Index >= _List->m_Size)
		return -1;

	if(_NodePtr == NULL)
		return -2;

	if(_Index == 0)
	{
		LinkedList_Node* item = LinkedList_UnlinkFirst(_List);
		*(_NodePtr) = item;
		return 0;
	}
	else if(_Index == _List->m_Size - 1)
	{
		LinkedList_Node* item = LinkedList_UnlinkLast(_List);
		*(_NodePtr) = item;
		return 0;
	}
	
	int index = 0;
	LinkedList_Node* currentNode = _List->m_Head;
	while(currentNode != NULL)
	{
		if(_Index == index)
		{
			

			currentNode->m_Privios->m_Next = currentNode->m_Next;
			currentNode->m_Next->m_Privios = currentNode->m_Privios;
			_List->m_Size--;

			currentNode->m_Next = NULL;
			currentNode->m_Privios = NULL;
			
			*(_NodePtr) = currentNode;
			
			return 0;
		}
		currentNode = currentNode->m_Next;
		index++;
	}

	return -3;//This will never happen!
}

int LinkedList_UnlinkItem(LinkedList* _List, void* _Item, LinkedList_Node** _NodePtr)
{
	if(_NodePtr == NULL)
		return -1;

	LinkedList_Node* currentNode = _List->m_Head;
	while(currentNode != NULL)
	{
		if(currentNode->m_Item == _Item)
		{
			if(currentNode == _List->m_Head)
			{
				LinkedList_Node* node = LinkedList_UnlinkFirst(_List);
				*(_NodePtr) = node;
				return 0;
			}
			else if(currentNode == _List->m_Tail)
			{
				LinkedList_Node* node = LinkedList_UnlinkLast(_List);
				*(_NodePtr) = node;
				return 0;
			}
			

			currentNode->m_Privios->m_Next = currentNode->m_Next;
			currentNode->m_Next->m_Privios = currentNode->m_Privios;
			_List->m_Size--;

			currentNode->m_Next = NULL;
			currentNode->m_Privios = NULL;
			*(_NodePtr) = currentNode;
			
			return 0;
		}
		currentNode = currentNode->m_Next;
	}

	return 1;
}

LinkedList_Node* LinkedList_UnlinkLast(LinkedList* _List)
{
	LinkedList_Node* tail = _List->m_Tail;

	if(_List->m_Size == 0)
		return NULL;

	if(_List->m_Size == 1)
		return LinkedList_UnlinkFirst(_List);

	_List->m_Tail = tail->m_Privios;
	_List->m_Tail->m_Next = NULL;
	_List->m_Size--;

	tail->m_Next = NULL;
	tail->m_Privios = NULL;

	return tail;
}

void LinkedList_UnlinkNode(LinkedList* _List, LinkedList_Node* _Node)
{
	if(_List->m_Head == _Node)
	{
		LinkedList_UnlinkFirst(_List);
		return;
	}

	else if(_List->m_Tail == _Node)
	{
		LinkedList_UnlinkLast(_List);
		return;
	}

	_Node->m_Privios->m_Next = _Node->m_Next;
	_Node->m_Next->m_Privios = _Node->m_Privios;

	_Node->m_Next = NULL;
	_Node->m_Privios = NULL;

	_List->m_Size--;
	
}

int LinkedList_CreateNode(void* _Item, LinkedList_Node** _NodePtr)
{
	if(_NodePtr == NULL)
		return -2;

	LinkedList_Node* _Node = (LinkedList_Node*) Allocator_Malloc(sizeof(LinkedList_Node));

	if(_Node == NULL)
		return -1;

	_Node->m_Item = _Item;
	_Node->m_Next = NULL;		
	_Node->m_Privios = NULL;

	*(_NodePtr) = _Node;

	return 0;
}

void LinkedList_Clear(LinkedList* _List)
{
	if(_List->m_Size == 0) {
		_List->m_Head = NULL;
		_List->m_Tail = NULL;
		return;

	}

	LinkedList_Node* currentNode = _List->m_Head;
	while(currentNode != NULL)
	{
		LinkedList_Node* disposedNode = currentNode;
		currentNode = currentNode->m_Next;
		Allocator_Free(disposedNode);
	}

	_List->m_Head = NULL;
	_List->m_Tail = NULL;
	_List->m_Size = 0;
}

void LinkedList_Dispose(LinkedList* _List)
{
	LinkedList_Clear(_List);
	

	if(_List->m_Allocated == True)
		Allocator_Free(_List);
	else
		memset(_List, 0, sizeof(LinkedList));

}