#ifndef Filesystem_Service_h__
#define Filesystem_Service_h__

struct T_Filesystem_Service;
typedef struct T_Filesystem_Service Filesystem_Service;

#define Filesystem_Service_VERSION 1u

#ifndef Filesystem_Service_BufferMax
	#define Filesystem_Service_BufferMax 1024
#endif

#include "../String.h"
#include "../File.h"
#include "../Folder.h"
#include "../Json.h"
#include "../Portability.h"
#include "../StateMachine.h"
#include "../TCP/TCPServer.h"
#include "../TCP/TCPClient.h"
#include "../Buffer.h"
#include "../Communication/DataLayer.h"
#include "../Communication/NetworkLayer.h"
#include "../Communication/TransportLayer.h"
#include "../EventHandler.h"

#include "Filesystem_Connection.h"
#include "Filesystem_Client.h"
#include "Filesystem_Server.h"

typedef struct
{
	String m_IP;
	UInt16 m_Port;
	
} Filesystem_ServiceSettings_Host;

typedef struct
{

	UInt16 m_Host;
	UInt16 m_Distributer;

	json_t* m_Servers; //! This will be removed later

	Bool m_AutoSync;
	UInt16 m_Interval;

} Filesystem_ServiceSettings;

struct T_Filesystem_Service
{
	Bool m_Allocated;
	StateMachine* m_Worker;
	StateMachine_Task* m_Task;
	
	json_t* m_Json;
	
	String m_Path;

	Filesystem_Server* m_Server;
	Filesystem_Client* m_Client;
	
	Buffer m_Buffer;

	Filesystem_ServiceSettings m_Settings;

	EventHandler m_EventHandler;
};

int Filesystem_Service_InitializePtr(StateMachine* _Worker, const char* _Path, Filesystem_Service** _ServicePtr);
int Filesystem_Service_Initialize(Filesystem_Service* _Service, StateMachine* _Worker, const char* _Path);

int Filesystem_Service_TCPRead(Filesystem_Service* _Service, LinkedList* _List, Buffer* _Buffer, int _Size);
int Filesystem_Service_TCPWrite(Filesystem_Service* _Service, LinkedList* _List, Buffer* _Buffer, int _Size);

void Filesystem_Service_Dispose(Filesystem_Service* _Service);
#endif // Filesystem_Service_h__