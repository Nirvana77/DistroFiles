#ifndef StateMachine_h__
#define StateMachine_h__

struct T_StateMachine;
typedef struct T_StateMachine StateMachine;

#include "LinkedList.h"
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <time.h>

typedef struct
{
    unsigned int m_Prio;
    void (*m_Callback)(UInt64 _MSTime, void* _Context);
    void* m_Context;

} StateMachine_Task;

struct T_StateMachine
{
    Bool m_Allocated;
    LinkedList m_List;

    LinkedList_Node* m_Current;
    
};

int StateMachine_InitializePtr(StateMachine** _StateMachinePtr);
int StateMachine_Initialize(StateMachine* _StateMachine);

int StateMachine_CreateTask(StateMachine* _StateMachine, unsigned int _Prio, const char* _Name, void (*_Callback)(UInt64 _MSTime, void* _Context), void* _Context, StateMachine_Task** _TaskPtr);
int StateMachine_RemoveTask(StateMachine* _StateMachine, StateMachine_Task* _Task);

void StateMachine_Dispose(StateMachine* _StateMachine);

#endif // StateMachine_h__