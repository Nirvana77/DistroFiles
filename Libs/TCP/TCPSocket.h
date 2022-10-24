#ifndef TCPSocket_h__
#define TCPSocket_h__

struct T_TCPSocket;
typedef struct T_TCPSocket TCPSocket;

#include "../Buffer.h"

#ifdef __linux__

	#include <unistd.h>
	#include <sys/fcntl.h>
	#include <arpa/inet.h>

	#define TCPSocket_Error -1
	
	typedef int TCPSocket_FD;

	static inline void TCPSocket_SetNonBlocking(TCPSocket_FD _FD)
	{
		int flags = fcntl(_FD, F_GETFL, 0);
		fcntl(_FD, F_SETFL, flags | O_NONBLOCK);
	}
#else

#endif

typedef enum
{
	TCPSocket_Status_Error = -2,
	TCPSocket_Status_Failed = -1,
	TCPSocket_Status_Init = 0,
	TCPSocket_Status_Connected = 1,
	TCPSocket_Status_Connecting = 2,
	TCPSocket_Status_Disconnected = 3,
	TCPSocket_Status_Closed= 4,
	
} TCPSocket_Status;

struct T_TCPSocket
{
	Bool m_Allocated;
	TCPSocket_FD m_FD;

	TCPSocket_Status m_Status;
	
	struct sockaddr_in m_Addr;
};

int TCPSocket_InitializePtr(const char* _IP, int _Port, TCPSocket_FD* _FD, TCPSocket** _TCPSocketPtr);
int TCPSocket_Initialize(TCPSocket* _TCPSocket, const char* _IP, int _Port, TCPSocket_FD* _FD);

int TCPSocket_Read(TCPSocket* _TCPSocket, Buffer* _Buffer, unsigned int _BufferSize);
int TCPSocket_Write(TCPSocket* _TCPSocket, Buffer* _Buffer, unsigned int _BufferSize);

void TCPSocket_Dispose(TCPSocket* _TCPSocket);


#endif // TCPSocket_h__