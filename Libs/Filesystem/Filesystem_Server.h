#ifndef Filesystem_Server_h__
#define Filesystem_Server_h__

struct T_Filesystem_Server;
typedef struct T_Filesystem_Server Filesystem_Server;

#include "Filesystem_Service.h"
#include "../BitHelper.h"
#include "../Memory.h"

#define Filesystem_Server_TempFlag_HasList 0
#define Filesystem_Server_TempFlag_WorkonList 1
#define Filesystem_Server_TempFlag_WillSend 2
#define Filesystem_Server_TempFlag_WillClear 3

#define Filesystem_Server_SyncTimeout 10000

typedef enum
{
	Filesystem_Server_State_Init = 0, // Initializeing
	Filesystem_Server_State_Idel = 1, // Ideling
	Filesystem_Server_State_Connecting = 2, // Connecting to servers
	Filesystem_Server_State_Checking = 3, // Checking if the acction is good or not. Success < 50% do ReSync else Synced
	Filesystem_Server_State_Synced = 4, // Synced with  servers
	Filesystem_Server_State_Syncing = 5, // Syncing with servers
	Filesystem_Server_State_ReSync = 6, // Sends sync message.
	Filesystem_Server_State_ReSyncing = 7, // Sends sync message.
} Filesystem_Server_State;

typedef enum
{
	Filesystem_Server_CheckState_None = 0,
	Filesystem_Server_CheckState_Write = 1,
	Filesystem_Server_CheckState_Delete = 2
} Filesystem_Server_CheckState;

typedef struct
{
	Bool m_IsUsed;
	Filesystem_Connection* m_Connection;
	UInt8 m_IsOk;

} Filesystem_Server_Check;

struct T_Filesystem_Server
{
	Bool m_Allocated;
	Filesystem_Service* m_Service;
	Filesystem_Server_State m_State;
	UInt64 m_LastSynced;
	
	String m_FilesytemPath;

	TCPServer m_TCPServer;
	DataLayer m_DataLayer;
	NetworkLayer m_NetworkLayer;
	TransportLayer m_TransportLayer;
	
	LinkedList m_Connections;
	LinkedList m_WriteChecked;
	Filesystem_Server_CheckState m_CheckState;
	
	UInt64 m_NextCheck;
	UInt64 m_Timeout;

	Byte m_TempFlag;
	Buffer m_TempListBuffer;
	UInt16 m_TempListSize;
	
};

int Filesystem_Server_InitializePtr(Filesystem_Service* _Service, Filesystem_Server** _ServerPtr);
int Filesystem_Server_Initialize(Filesystem_Server* _Server, Filesystem_Service* _Service);

void Filesystem_Server_Work(UInt64 _MSTime, Filesystem_Server* _Server);

static inline Bool Filesystem_Server_HashCheck(unsigned char _A[16], unsigned char _B[16])
{
	for (int i = 0; i < 16; i++)
	{
		if(_A[i] != _B[i])
			return False;
	}
	
	return True;
}

static inline void Filesystem_Server_GetTimeFromPath(char* _Path, UInt64* _Value)
{
	struct stat attr;
	char str[64];
	UInt64 value = 0;
	stat(_Path, &attr);
	sprintf(str, "%u", attr.st_mtim);

	for (int i = 0; i < strlen(str); i++)
	{
		value += str[i] - 48;
		value *= 10;
	}
	value /= 10;
	*(_Value) = value;
}

static inline void Filesystem_Server_PrintHash(const char* _Name, unsigned char _Result[16])
{

	printf("%s: ", _Name);

	for (int i = 0; i < 16; i++)
		printf("%x", _Result[i]);
	
	printf("\r\n");
}

void Filesystem_Server_Sync(Filesystem_Server* _Server);

int Filesystem_Server_GetList(Filesystem_Server* _Server, char* _Path, Buffer* _DataBuffer);
int Filesystem_Server_Write(Filesystem_Server* _Server, Bool _IsFile, char* _Path, Buffer* _DataBuffer);
int Filesystem_Server_Delete(Filesystem_Server* _Server, Bool _IsFile, char* _Path);


void Filesystem_Server_Dispose(Filesystem_Server* _Server);

#endif // Filesystem_Server_h__
