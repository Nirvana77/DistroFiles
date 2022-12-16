#ifndef DistroFiles_Server_h__
#define DistroFiles_Server_h__

struct T_DistroFiles_Server;
typedef struct T_DistroFiles_Server DistroFiles_Server;

#include "DistroFiles_Service.h"
#include "DistroFiles_Checking.h"
#include "../BitHelper.h"
#include "../Memory.h"
#include "../Communication/Bus.h"

#define DistroFiles_Server_TempFlag_HasList 0
#define DistroFiles_Server_TempFlag_WorkonList 1
#define DistroFiles_Server_TempFlag_WillSend 2
#define DistroFiles_Server_TempFlag_WillClear 3

#ifndef DistroFiles_Server_SyncTimeout
	#define DistroFiles_Server_SyncTimeout (SEC*10)
#endif
#ifndef DistroFiles_Server_SyncErrorTimeout
	#define DistroFiles_Server_SyncErrorTimeout (SEC)
#endif
typedef enum
{
	DistroFiles_Server_State_Init = 0, // Initializeing
	DistroFiles_Server_State_Idel = 1, // Ideling
	DistroFiles_Server_State_Connecting = 2, // Connecting to servers
	DistroFiles_Server_State_Conneced = 3, // Connected to servers
	DistroFiles_Server_State_Checking = 4, // Checking if the acction is good or not. Success < 50% do ReSync else Synced
	DistroFiles_Server_State_Synced = 5, // Synced with  servers
	DistroFiles_Server_State_Syncing = 6, // Syncing with servers
	DistroFiles_Server_State_ReSync = 7, // Sends sync message.
	DistroFiles_Server_State_ReSyncing = 8, // Sends sync message.
	DistroFiles_Server_State_SyncError = 9, //Error when doing Resyncing or Syncing
} DistroFiles_Server_State;

const char* DistroFiles_Server_States[] = {
	"DistroFiles_Server_State_Init",
	"DistroFiles_Server_State_Idel",
	"DistroFiles_Server_State_Connecting",
	"DistroFiles_Server_State_Conneced",
	"DistroFiles_Server_State_Checking",
	"DistroFiles_Server_State_Synced",
	"DistroFiles_Server_State_Syncing",
	"DistroFiles_Server_State_ReSync",
	"DistroFiles_Server_State_ReSyncing",
	"DistroFiles_Server_State_SyncError"
};

typedef enum
{
	DistroFiles_Server_Event_Update = 0
} DistroFiles_Server_Event;

struct T_DistroFiles_Server
{
	Bool m_Allocated;
	DistroFiles_Service* m_Service;
	DistroFiles_Server_State m_State;
	UInt64 m_LastSynced;
	
	String m_FilesytemPath;

	Bus m_Bus;
	TCPServer m_TCPServer;
	DataLayer m_DataLayer;
	NetworkLayer m_NetworkLayer;
	TransportLayer m_TransportLayer;
	
	LinkedList m_Connections;
	DistroFiles_Checking m_Checking;
	
	UInt64 m_NextCheck;
	UInt64 m_Timeout;
	UInt64 m_ErrorTimeout;

	Byte m_TempFlag;
	Buffer m_TempListBuffer;
	UInt16 m_TempListSize;
	
	EventHandler m_EventHandler;
};

int DistroFiles_Server_InitializePtr(DistroFiles_Service* _Service, DistroFiles_Server** _ServerPtr);
int DistroFiles_Server_Initialize(DistroFiles_Server* _Server, DistroFiles_Service* _Service);

void DistroFiles_Server_Work(UInt64 _MSTime, DistroFiles_Server* _Server);

static inline Bool DistroFiles_Server_HashCheck(unsigned char _A[16], unsigned char _B[16])
{
	for (int i = 0; i < 16; i++)
	{
		if(_A[i] != _B[i])
			return False;
	}
	
	return True;
}

static inline void DistroFiles_Server_GetTimeFromPath(char* _Path, UInt64* _Value)
{
	struct stat attr;
	stat(_Path, &attr);
	*(_Value) = (UInt64)attr.st_mtim.tv_sec;;
}

static inline void DistroFiles_Server_PrintHash(const char* _Name, unsigned char _Result[16])
{

	printf("%s: ", _Name);

	for (int i = 0; i < 16; i++)
		printf("%x", _Result[i]);
	
	printf("\r\n");
}

int DistroFiles_Server_Sync(DistroFiles_Server* _Server, Payload** _MessagePtr);

int DistroFiles_Server_GetList(DistroFiles_Server* _Server, char* _Path, Buffer* _DataBuffer);
int DistroFiles_Server_Write(DistroFiles_Server* _Server, Bool _IsFile, char* _Path, Buffer* _DataBuffer);
int DistroFiles_Server_Delete(DistroFiles_Server* _Server, Bool _IsFile, char* _Path);


void DistroFiles_Server_Dispose(DistroFiles_Server* _Server);

#endif // DistroFiles_Server_h__
