#ifndef Filesystem_Checking_h__
#define Filesystem_Checking_h__

struct T_Filesystem_Checking;
typedef struct T_Filesystem_Checking Filesystem_Checking;

#include "Filesystem_Server.h"

typedef enum
{
	Filesystem_Checking_Type_None = 0,
	Filesystem_Checking_Type_Write = 1,
	Filesystem_Checking_Type_Delete = 2,
	Filesystem_Checking_Type_Syncing = 3
} Filesystem_Checking_Type;

typedef enum
{
	Filesystem_Checking_Check_Satus_OK = 0,
	Filesystem_Checking_Check_Satus_NOTOK = 1,
	Filesystem_Checking_Check_Satus_DontHave = 2,
} Filesystem_Checking_Check_Satus;

typedef struct
{
	Bool m_IsUsed;
	Filesystem_Connection* m_Connection;
	Filesystem_Checking_Check_Satus m_IsOk;

} Filesystem_Checking_Check;

struct T_Filesystem_Checking
{
	Bool m_Allocated;
	Filesystem_Checking_Type m_Type;
	Filesystem_Server* m_Server;
	Payload m_Message;
	LinkedList m_List;
};

int Filesystem_Checking_InitializePtr(Filesystem_Server* _Server,Filesystem_Checking** _CheckingPtr);
int Filesystem_Checking_Initialize(Filesystem_Checking* _Checking, Filesystem_Server* _Server);

static inline void Filesystem_Checking_SetState(Filesystem_Checking* _Checking, Filesystem_Checking_Type _Type, Payload* _Message)
{
	printf("Set Checking_State to %i\r\n", (int)_Type);
	Payload_Copy(&_Checking->m_Message, _Message);
	_Checking->m_Type = _Type;
}

void Filesystem_Checking_RemoveCheck(Filesystem_Checking* _Checking, Filesystem_Checking_Check* _Check);
void Filesystem_Checking_ClearWriteCheckList(Filesystem_Checking* _Checking);
int Filesystem_Checking_SpawnWriteCheck(Filesystem_Checking* _Checking, Payload_Address* _Address, Filesystem_Checking_Check** _CheckPtr);
int Filesystem_Checking_WorkOnPayload(Filesystem_Checking* _Checking, Filesystem_Checking_Type _Type, Payload* _Message);

Bool Filesystem_Checking_CanUseConnection(Filesystem_Checking* _Checking, Filesystem_Connection* _Connection);

void Filesystem_Checking_Dispose(Filesystem_Checking* _Checking);

#endif // Filesystem_Checking_h__