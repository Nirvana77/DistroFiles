#ifndef TCPSocket_h__
#define TCPSocket_h__

struct T_TCPSocket;
typedef struct T_TCPSocket TCPSocket;

#include "../Buffer.h"

#ifdef __linux__

	#include <unistd.h>
	#include <sys/fcntl.h>
	#include <arpa/inet.h>

	#include <sys/types.h>
	#include <sys/socket.h>
	#include <sys/ioctl.h>
	#include <netinet/in.h>
	#include <net/if.h>

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

static inline void GetIP(UInt8 _Address[4])
{
	int n;
	struct ifreq ifr;
	char* array = "eth0";

	n = socket(AF_INET, SOCK_DGRAM, 0);
	//Type of address to retrieve - IPv4 IP address
	ifr.ifr_addr.sa_family = AF_INET;
	//Copy the interface name in the ifreq structure
	strncpy(ifr.ifr_name , array , IFNAMSIZ - 1);
	ioctl(n, SIOCGIFADDR, &ifr);
	close(n);
	//display result
	char str[16] = "";
	sprintf(str, "%s", inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr) );

	int index = 0;
	UInt8 value = 0;
	for (int i = 0; i < strlen(str); i++)
	{
		if(str[i] == '.')
		{
			_Address[index++] = value;
			value = 0;
		}
		else
		{
			if(value > 10 || i == 1 || i == 5 || i == 9 || i == 13)
				value *= 10;

			value += (int)str[i] - 48;
		}
	}
	
	_Address[index++] = value;
}

static inline int GetMAC(UInt8 _Address[6])
{
	struct ifreq s;
	int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);

	strcpy(s.ifr_name, "eth0");
	if (0 == ioctl(fd, SIOCGIFHWADDR, &s)) {
		int j = 0;
		for (int i = 0; i < 6; ++i)
			_Address[j++] =s.ifr_addr.sa_data[i] - 48;
		return 0;
	}

	return 1;
}

#endif // TCPSocket_h__