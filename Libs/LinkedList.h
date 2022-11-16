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

int LinkedList_CreateNode(void* _Item, LinkedList_Node** _ElementPtr);

int LinkedList_LinkFirst(LinkedList* _List, LinkedList_Node* _Node);
int LinkedList_LinkLast(LinkedList* _List, LinkedList_Node* _Node);

static inline int LinkedList_AddFirst(LinkedList* _List, void* _Item)
{
	LinkedList_Node* node = NULL;
	if(LinkedList_CreateNode(_Item, &node) != 0)
		return -1;

    return LinkedList_LinkLast(_List, node);
}
static inline int LinkedList_AddLast(LinkedList* _List, void* _Item)
{
	LinkedList_Node* node = NULL;
	if(LinkedList_CreateNode(_Item, &node) != 0)
		return -1;

    return LinkedList_LinkLast(_List, node);
}

#define LinkedList_Push(a,b) ((int)LinkedList_AddLast(a,b))

LinkedList_Node* LinkedList_UnlinkFirst(LinkedList* _List);
int LinkedList_UnlinkAt(LinkedList* _List, int _Index, LinkedList_Node** _NodePtr);
int LinkedList_UnlinkItem(LinkedList* _List, void* _Item, LinkedList_Node** _NodePtr);
LinkedList_Node* LinkedList_UnlinkLast(LinkedList* _List);
void LinkedList_UnlinkNode(LinkedList* _List, LinkedList_Node* _Node);

static inline void* LinkedList_RemoveFirst(LinkedList* _List)
{
    LinkedList_Node* node = LinkedList_UnlinkFirst(_List);

    if(node == NULL)
        return NULL;

    void* item = node->m_Item;
    Allocator_Free(node);
    return item;
}
static inline int LinkedList_RemoveAt(LinkedList* _List, int _Index, void** _ItemPtr)
{
    LinkedList_Node* node = NULL;
    int success = LinkedList_UnlinkAt(_List, _Index, &node);

    if(success < 0)
        return success;

    if(_ItemPtr != NULL)
        *(_ItemPtr) = node->m_Item;

    if(success == 0)
        Allocator_Free(node);

    return 0;
}
static inline int LinkedList_RemoveItem(LinkedList* _List, void* _Item)
{
    LinkedList_Node* node = NULL;
    
    int success = LinkedList_UnlinkItem(_List, _Item, &node);
    
    if(success == 0)
        Allocator_Free(node);

    return success;
}
static inline void* LinkedList_RemoveLast(LinkedList* _List)
{
    LinkedList_Node* node = LinkedList_UnlinkLast(_List);

    if(node == NULL)
        return NULL;
    
    void* item = node->m_Item;
    Allocator_Free(node);
    return item;
}
static inline void* LinkedList_RemoveNode(LinkedList* _List, LinkedList_Node* _Node)
{
    void* item = _Node->m_Item;
    LinkedList_UnlinkNode(_List, _Node);
    Allocator_Free(_Node);
    return item;
}

void LinkedList_Clear(LinkedList* _List);


void LinkedList_Dispose(LinkedList* _List);


#endif // LinkedList_h__
