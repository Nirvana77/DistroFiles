#ifndef LinkedList_h__
#define LinkedList_h__

struct T_LinkedList;
typedef struct T_LinkedList LinkedList;

struct T_LinkedList_Node;
typedef struct T_LinkedList_Node LinkedList_Node;

#include "Allocator.h"
#include "Portability.h"

struct T_LinkedList_Node
{
    LinkedList_Node* m_Next;
    LinkedList_Node* m_Privios;
	void* m_Item;
	
};

struct T_LinkedList
{
	Bool m_Allocated;
	
    LinkedList_Node* m_Head;
    LinkedList_Node* m_Tail;
	int m_Size;
	
};

int LinkedList_InitializePtr(LinkedList** _ListPtr);
int LinkedList_Initialize(LinkedList* _List);


int LinkedList_AddFirst(LinkedList* _List, void* _Item);
int LinkedList_AddLast(LinkedList* _List, void* _Item);

#define LinkedList_Push(a,b) ((int)LinkedList_AddLast(a,b))

void* LinkedList_RemoveFirst(LinkedList* _List);
void* LinkedList_RemoveAt(LinkedList* _List, int _Index);
int LinkedList_RemoveItem(LinkedList* _List, void* _Item);
void* LinkedList_RemoveLast(LinkedList* _List);
void* LinkedList_RemoveNode(LinkedList* _List, LinkedList_Node* _Node);

void LinkedList_Clear(LinkedList* _List);


void LinkedList_Dispose(LinkedList* _List);


#endif // LinkedList_h__
