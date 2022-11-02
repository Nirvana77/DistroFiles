#ifndef Filesystem_Client_h__
#define Filesystem_Client_h__

struct T_Filesystem_Client;
typedef struct T_Filesystem_Client Filesystem_Client;

#include "Filesystem_Service.h"

struct T_Filesystem_Client
{
	Bool m_Allocated;
	Filesystem_Service* m_Service;

	TCPClient m_TCPClient;
	DataLayer m_DataLayer;
	NetworkLayer m_NetworkLayer;
	TransportLayer m_TransportLayer;

};

int Filesystem_Client_InitializePtr(Filesystem_Service* _Service, Filesystem_Client** _ClientPtr);
int Filesystem_Client_Initialize(Filesystem_Client* _Client, Filesystem_Service* _Service);

int Filesystem_Client_SendMessage(Filesystem_Client* _Client, unsigned char* _Data, int _Size);

void Filesystem_Client_Work(UInt64 _MSTime, Filesystem_Client* _Client);

void Filesystem_Client_Dispose(Filesystem_Client* _Client);
#endif // Filesystem_Client_h__