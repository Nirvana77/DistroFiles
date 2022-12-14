#include "TCPSocket.h"

int TCPSocket_InitializePtr(const char* _IP, int _Port, TCPSocket_FD* _FD, TCPSocket** _TCPSocketPtr)
{
	TCPSocket* _TCPSocket = (TCPSocket*)Allocator_Malloc(sizeof(TCPSocket));
	if(_TCPSocket == NULL)
		return -1;
	
	int success = TCPSocket_Initialize(_TCPSocket, _IP, _Port, _FD);
	if(success != 0)
	{
		Allocator_Free(_TCPSocket);
		return success;
	}
	
	_TCPSocket->m_Allocated = True;
	
	*(_TCPSocketPtr) = _TCPSocket;
	return 0;
}

int TCPSocket_Initialize(TCPSocket* _TCPSocket, const char* _IP, int _Port, TCPSocket_FD* _FD)
{
	_TCPSocket->m_Allocated = False;
	
	if(_FD == NULL)
	{
		_TCPSocket->m_FD = socket(AF_INET, SOCK_STREAM, 0); //? IPPROTO_TCP?
		_TCPSocket->m_Status = TCPSocket_Status_Disconnected;

		if(_TCPSocket->m_FD < 0)
		{
			printf("Socket error\n\r");
			_TCPSocket->m_Status = TCPSocket_Status_Error;
			return -2;
		}

		memset(&_TCPSocket->m_Addr, 0, sizeof(_TCPSocket->m_Addr));
		_TCPSocket->m_Addr.sin_family = AF_INET;
		_TCPSocket->m_Addr.sin_port = htons(_Port);
		_TCPSocket->m_Addr.sin_addr.s_addr = inet_addr(_IP);

		TCPSocket_SetNonBlocking(_TCPSocket->m_FD);
		connect(_TCPSocket->m_FD, (struct sockaddr*)&_TCPSocket->m_Addr,  sizeof(struct sockaddr_in));

		_TCPSocket->m_Status = TCPSocket_Status_Connected;

	}
	else
	{
		_TCPSocket->m_FD = *(_FD);
		TCPSocket_SetNonBlocking(_TCPSocket->m_FD);
		_TCPSocket->m_Status = TCPSocket_Status_Connected;
	}

	return 0;
}

int TCPSocket_Read(void* _Context, unsigned char* _Buffer, int _Size, TCPSocket_Error* _Error)
{
	TCPSocket* _TCPSocket = (TCPSocket*) _Context;
	_Error->m_HasError = False;
	int bytesRead = recv(_TCPSocket->m_FD, _Buffer, _Size, 0);

	if(bytesRead < 0)
	{
		if(_Error != NULL)
		{
			_Error->m_HasError = True;
			_Error->m_Error = errno;
		}
		if (errno == EWOULDBLOCK)
		{
			return bytesRead;
		}
		else if(errno == 0)
		{
			_TCPSocket->m_Status = TCPSocket_Status_Connected;
			return bytesRead;
		}
		else if(errno == 32 || errno == ENOTSOCK)
		{
			_TCPSocket->m_Status = TCPSocket_Status_Closed;
			return bytesRead;
		}
		else 
		{
			printf("TCPSocket Read Error %i\r\n", errno);
			_TCPSocket->m_Status = TCPSocket_Status_Failed;
			return 0;
		}
	}
	else if(bytesRead == 0)
	{
		_TCPSocket->m_Status = TCPSocket_Status_Closed;
		return 0;
	}
	else
	{
		return bytesRead;
	}

}

int TCPSocket_Write(void* _Context, unsigned char* _Buffer, int _Size, TCPSocket_Error* _Error)
{
	TCPSocket* _TCPSocket = (TCPSocket*) _Context;
	int bytesSent = send(_TCPSocket->m_FD, _Buffer, _Size, MSG_NOSIGNAL);

	if(bytesSent < 0)
	{
		if(_Error != NULL)
		{
			_Error->m_HasError = True;
			_Error->m_Error = errno;
		}

		if (errno == EWOULDBLOCK)
		{
			return bytesSent;
		}
		else if(errno == 0)
		{
			_TCPSocket->m_Status = TCPSocket_Status_Connected;
			return bytesSent;
		}
		else if(errno == 32)
		{
			_TCPSocket->m_Status = TCPSocket_Status_Closed;
			return bytesSent;
		}
		else 
		{
			printf("TCPSocket Write Error %i\r\n", errno);
			_TCPSocket->m_Status = TCPSocket_Status_Failed;
			return 0;
		}
	}

	return bytesSent;
}

void TCPSocket_Disconnect(TCPSocket* _TCPSocket)
{
	if(_TCPSocket->m_Status == TCPSocket_Status_Closed || _TCPSocket->m_FD == 0)
		return;

	#ifdef __linux__
		close(_TCPSocket->m_FD);
	#else

	#endif

	_TCPSocket->m_Status = TCPSocket_Status_Closed;
}

void TCPSocket_Dispose(TCPSocket* _TCPSocket)
{
	TCPSocket_Disconnect(_TCPSocket);

	if(_TCPSocket->m_Allocated == True)
		Allocator_Free(_TCPSocket);
	else
		memset(_TCPSocket, 0, sizeof(TCPSocket));

}
