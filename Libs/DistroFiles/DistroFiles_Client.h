#ifndef DistroFiles_Client_h__
#define DistroFiles_Client_h__

struct T_DistroFiles_Client;
typedef struct T_DistroFiles_Client DistroFiles_Client;


#include "DistroFiles_Service.h"

struct T_DistroFiles_Client
{
	Bool m_Allocated;
	DistroFiles_Service* m_Service;
	DistroFiles_Server* m_Server;

	UInt64 m_Timeout;
	UInt64 m_NextCheck;

	LinkedList m_Connections;
	EventHandler_Event* m_Hook;

	Bus m_Bus;
	TCPServer m_TCPServer;
	DataLayer m_DataLayer;
	NetworkLayer m_NetworkLayer;
	TransportLayer m_TransportLayer;

	EventHandler m_EventHandler;

};

int DistroFiles_Client_InitializePtr(DistroFiles_Service* _Service, DistroFiles_Client** _ClientPtr);
int DistroFiles_Client_Initialize(DistroFiles_Client* _Client, DistroFiles_Service* _Service);

int DistroFiles_Client_SendMessage(DistroFiles_Client* _Client, unsigned char* _Data, int _Size);

void DistroFiles_Client_Work(UInt64 _MSTime, DistroFiles_Client* _Client);

void DistroFiles_Client_Dispose(DistroFiles_Client* _Client);
#endif // DistroFiles_Client_h__