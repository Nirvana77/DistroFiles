#ifndef StateMachine_h__
#define StateMachine_h__

struct T_StateMachine;
typedef struct T_StateMachine StateMachine;

#include "LinkedList.h"
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>

typedef struct
{
    pthread_t m_Thread;
    int (*m_Callback)(UInt64 _MSTime, void* _Context);
    void* m_Context;
    Bool m_Disposed;

} StateMachine_Task;

struct T_StateMachine
{
    Bool m_Allocated;
    LinkedList m_List;
    
};

int StateMachine_InitializePtr(StateMachine** _StateMachinePtr);
int StateMachine_Initialize(StateMachine* _StateMachine);

//* Then the callback returns 1 the task will be desposed
int StateMachine_CreateTask(StateMachine* _StateMachine, pthread_attr_t* _Attr, const char* _Name, int (*_Callback)(UInt64 _MSTime, void* _Context), void* _Context, StateMachine_Task** _TaskPtr);
int StateMachine_RemoveTask(StateMachine* _StateMachine, StateMachine_Task* _Task);

void StateMachine_Dispose(StateMachine* _StateMachine);

#endif // StateMachine_h__