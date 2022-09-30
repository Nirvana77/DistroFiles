#ifndef Filesystem_Server_h__
#define Filesystem_Server_h__

struct T_Filesystem_Server;
typedef struct T_Filesystem_Server Filesystem_Server;

struct T_Filesystem_Server
{
	Bool m_Allocated;
	
};

int Filesystem_Server_InitializePtr(const char* _Path, Filesystem_Server** _ServerPtr);
int Filesystem_Server_Initialize(Filesystem_Server* _Server, const char* _Path);
void Filesystem_Server_Dispose(Filesystem_Server* _Server);

#endif // Filesystem_Server_h__
