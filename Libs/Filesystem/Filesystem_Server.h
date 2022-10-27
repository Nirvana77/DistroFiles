#ifndef Filesystem_Server_h__
#define Filesystem_Server_h__

struct T_Filesystem_Server;
typedef struct T_Filesystem_Server Filesystem_Server;

#include "Filesystem_Service.h"
#include "../BitHelper.h"

#define Filesystem_Server_TempFlag_HasList 0
#define Filesystem_Server_TempFlag_WorkonList 1
#define Filesystem_Server_TempFlag_WillSend 2
#define Filesystem_Server_TempFlag_WillClear 3

struct T_Filesystem_Server
{
	Bool m_Allocated;
	Filesystem_Service* m_Service;

	TCPServer m_TCPServer;
	DataLayer m_DataLayer;
	NetworkLayer m_NetworkLayer;
	TransportLayer m_TransportLayer;
	
	LinkedList m_Sockets;
	LinkedList_Node* m_CurrentNode;
	Buffer m_Buffer;

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

void Filesystem_Server_Dispose(Filesystem_Server* _Server);

#endif // Filesystem_Server_h__
