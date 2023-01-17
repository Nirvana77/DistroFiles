#ifndef DistroFiles_Checking_h__
#define DistroFiles_Checking_h__

struct T_DistroFiles_Checking;
typedef struct T_DistroFiles_Checking DistroFiles_Checking;

#include "DistroFiles_Server.h"

#ifndef DistroFiles_Checking_Timeout
	#define  DistroFiles_Checking_Timeout (SEC * 10)
#endif

#ifndef DistroFiles_Checking_CheckError
	#define DistroFiles_Checking_CheckError 50 //This is in %
#endif
typedef enum
{
	DistroFiles_Checking_Type_None = 0,
	DistroFiles_Checking_Type_Write = 1,
	DistroFiles_Checking_Type_Delete = 2,
	DistroFiles_Checking_Type_Syncing = 3
} DistroFiles_Checking_Type;

typedef enum
{
	DistroFiles_Checking_Check_Satus_OK = 0,
	DistroFiles_Checking_Check_Satus_NOTOK = 1,
	DistroFiles_Checking_Check_Satus_DontHave = 2,
} DistroFiles_Checking_Check_Satus;

typedef struct
{
	Bool m_IsUsed;
	DistroFiles_Connection* m_Connection;
	DistroFiles_Checking_Check_Satus m_IsOk;
	UInt64 m_Timeout;

} DistroFiles_Checking_Check;

struct T_DistroFiles_Checking
{
	Bool m_Allocated;
	DistroFiles_Checking_Type m_Type;
	DistroFiles_Server* m_Server;
	Payload m_Message;
	LinkedList m_List;
};

int DistroFiles_Checking_InitializePtr(DistroFiles_Server* _Server,DistroFiles_Checking** _CheckingPtr);
int DistroFiles_Checking_Initialize(DistroFiles_Checking* _Checking, DistroFiles_Server* _Server);

static inline void DistroFiles_Checking_SetState(DistroFiles_Checking* _Checking, DistroFiles_Checking_Type _Type, Payload* _Message)
{
	printf("Set Checking_State to %i\r\n", (int)_Type);
	Payload_DeepCopy(&_Checking->m_Message, _Message);
	_Checking->m_Type = _Type;
}

void DistroFiles_Checking_RemoveCheck(DistroFiles_Checking* _Checking, DistroFiles_Checking_Check* _Check);
void DistroFiles_Checking_ClearWriteCheckList(DistroFiles_Checking* _Checking);
int DistroFiles_Checking_SpawnWriteCheck(DistroFiles_Checking* _Checking, Payload_Address* _Address, DistroFiles_Checking_Check** _CheckPtr);
int DistroFiles_Checking_WorkOnPayload(DistroFiles_Checking* _Checking, DistroFiles_Checking_Type _Type, Payload* _Message);

Bool DistroFiles_Checking_CanUseConnection(DistroFiles_Checking* _Checking, DistroFiles_Connection* _Connection);

void DistroFiles_Checking_Work(UInt64 _MSTime, DistroFiles_Checking* _Checking);

void DistroFiles_Checking_Dispose(DistroFiles_Checking* _Checking);

#endif // DistroFiles_Checking_h__