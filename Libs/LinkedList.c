#include "LinkedList.h"
int LinkedList_CreateNode(void* _Item, LinkedList_Node** _ElementPtr);

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

int LinkedList_AddFirst(LinkedList* _List, void* _Item)
{
	LinkedList_Node* element = NULL;
	if(LinkedList_CreateNode(_Item, &element) != 0)
		return -1;
	

	if(_List->m_Size == 0)
	{
		_List->m_Head = element;
		_List->m_Tail = element;
		_List->m_Size = 1;
		return 0;
	}

	_List->m_Head->m_Privios = element;
	element->m_Next = _List->m_Head;
	_List->m_Head = element;
	_List->m_Size++;

	return 0;
}

int LinkedList_AddLast(LinkedList* _List, void* _Item)
{
	LinkedList_Node* element = NULL;
	if(LinkedList_CreateNode(_Item, &element) != 0)
		return -1;

	if(_List->m_Size == 0)
	{
		_List->m_Head = element;
		_List->m_Tail = element;
		_List->m_Size = 1;
		return 0;
	}
	
	_List->m_Tail->m_Next = element;
	element->m_Privios = _List->m_Tail;
	_List->m_Tail = element;
	_List->m_Size++;

	return 0;
}

void* LinkedList_RemoveFirst(LinkedList* _List)
{
	LinkedList_Node* head = _List->m_Head;

	if(_List->m_Size == 0)
		return NULL;

	void* item = head->m_Item;

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

	Allocator_Free(head);
	return item;
}

//TODO: #4 implement LinkedList_RemoveAt
void* LinkedList_RemoveAt(LinkedList* _List, int _Index)
{
	fprintf(stderr, "Not implemented\n\r");
	return NULL;
}

int LinkedList_RemoveItem(LinkedList* _List, void* _Item)
{

	LinkedList_Node* currentNode = _List->m_Head;
	while(currentNode != NULL)
	{
		if(currentNode->m_Item == _Item)
		{
			if(currentNode == _List->m_Head)
				return LinkedList_RemoveFirst(_List);
			
			else if(currentNode == _List->m_Tail)
				return LinkedList_RemoveLast(_List);
			

			currentNode->m_Privios->m_Next = currentNode->m_Next;
			currentNode->m_Next->m_Privios = currentNode->m_Privios;
			
			return 0;
		}
		currentNode = currentNode->m_Next;
	}

	return 1;
}

void* LinkedList_RemoveLast(LinkedList* _List)
{
	LinkedList_Node* tail = _List->m_Tail;

	if(_List->m_Size == 0)
		return NULL;

	if(_List->m_Size == 1)
		return LinkedList_RemoveFirst(_List);
	
	void* item = tail->m_Item;

	_List->m_Tail = tail->m_Privios;
	_List->m_Tail->m_Next = NULL;
	_List->m_Size--;

	Allocator_Free(tail);
	return item;
}

void* LinkedList_RemoveNode(LinkedList* _List, LinkedList_Node* _Node)
{
	if(_List->m_Head == _Node)
		return LinkedList_RemoveFirst(_List);

	else if(_List->m_Tail == _Node)
		return LinkedList_RemoveLast(_List);

	void* item = _Node->m_Item;
	_Node->m_Privios->m_Next = _Node->m_Next;
	_Node->m_Next->m_Privios = _Node->m_Privios;
	
	Allocator_Free(_Node);
	
	return item;
}

int LinkedList_CreateNode(void* _Item, LinkedList_Node** _ElementPtr)
{
	if(_ElementPtr == NULL)
		return -2;

	LinkedList_Node* _Element = (LinkedList_Node*) Allocator_Malloc(sizeof(LinkedList_Node));

	if(_Element == NULL)
		return -1;

	_Element->m_Item = _Item;
	_Element->m_Next = NULL;		
	_Element->m_Privios = NULL;

	*(_ElementPtr) = _Element;

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