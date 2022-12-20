#ifndef DistroFiles_Service_h__
#define DistroFiles_Service_h__

struct T_DistroFiles_Service;
typedef struct T_DistroFiles_Service DistroFiles_Service;

#define DistroFiles_Service_VERSION 1u

#define DistroFiles_DatalayerWorkTime (MS * 100)

#ifndef DistroFiles_Service_BufferMax
	#define DistroFiles_Service_BufferMax 1024
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

#include "DistroFiles_Connection.h"
#include "DistroFiles_Client.h"
#include "DistroFiles_Server.h"

typedef struct
{
	String m_IP;
	UInt16 m_Port;
	
} DistroFiles_ServiceSettings_Host;

typedef struct
{

	UInt16 m_Host;
	UInt16 m_Distributer;

	json_t* m_Servers; //! This will be removed later

	Bool m_AutoSync;
	UInt16 m_Interval;

} DistroFiles_ServiceSettings;

struct T_DistroFiles_Service
{
	Bool m_Allocated;
	StateMachine* m_Worker;
	StateMachine_Task* m_Task;
	
	json_t* m_Json;
	
	String m_Path;

	DistroFiles_Server* m_Server;
	DistroFiles_Client* m_Client;
	
	Buffer m_Buffer;

	DistroFiles_ServiceSettings m_Settings;

	EventHandler m_EventHandler;
};

int DistroFiles_Service_InitializePtr(StateMachine* _Worker, const char* _Path, DistroFiles_Service** _ServicePtr);
int DistroFiles_Service_Initialize(DistroFiles_Service* _Service, StateMachine* _Worker, const char* _Path);

int DistroFiles_Service_TCPRead(DistroFiles_Service* _Service, LinkedList* _List, Buffer* _Buffer, int _Size);
int DistroFiles_Service_TCPWrite(DistroFiles_Service* _Service, LinkedList* _List, Buffer* _Buffer, int _Size);

void DistroFiles_Service_Dispose(DistroFiles_Service* _Service);
#endif // DistroFiles_Service_h__