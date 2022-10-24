#ifndef Filesystem_Service_h__
#define Filesystem_Service_h__

struct T_Filesystem_Service;
typedef struct T_Filesystem_Service Filesystem_Service;

#define Filesystem_Service_VERSION 1u

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

#include "Filesystem_Client.h"
#include "Filesystem_Server.h"

typedef struct
{
	String m_IP;
	UInt16 m_Port;
	
} Filesystem_ServiceSettings_Host;

typedef struct
{

	Filesystem_ServiceSettings_Host m_Host;
	Filesystem_ServiceSettings_Host m_Distributer;

	json_t* m_Servers; //! This will be removed later

} Filesystem_ServiceSettings;

struct T_Filesystem_Service
{
	Bool m_Allocated;
	StateMachine* m_Worker;
	StateMachine_Task* m_Task;
	
	json_t* m_Json;
	
	String m_Path;
	String m_FilesytemPath;

	Filesystem_Server* m_Server;
	Filesystem_Client* m_Client;

	Filesystem_ServiceSettings m_Settings;

};

int Filesystem_Service_InitializePtr(StateMachine* _Worker, const char* _Path, Filesystem_Service** _ServicePtr);
int Filesystem_Service_Initialize(Filesystem_Service* _Service, StateMachine* _Worker, const char* _Path);


void Filesystem_Service_Dispose(Filesystem_Service* _Service);
#endif // Filesystem_Service_h__