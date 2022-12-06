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

int TCPSocket_Read(void* _Context, Buffer* _Buffer)
{
	TCPSocket* _TCPSocket = (TCPSocket*) _Context;
	int bytesRead = recv(_TCPSocket->m_FD, _Buffer->m_WritePtr, Buffer_SizeLeft(_Buffer), 0);

	if(bytesRead < 0)
	{
		_TCPSocket->m_Status = TCPSocket_Status_Error;
		return -1;
	}
	else if(bytesRead == 0)
	{
		_TCPSocket->m_Status = TCPSocket_Status_Closed;
		return 0;
	}
	else
	{
		_Buffer->m_BytesLeft = bytesRead;
		return bytesRead;
	}

}

int TCPSocket_Write(void* _Context, Buffer* _Buffer)
{
	TCPSocket* _TCPSocket = (TCPSocket*) _Context;
	int bytesSent = send(_TCPSocket->m_FD, _Buffer->m_ReadPtr, _Buffer->m_BytesLeft, MSG_NOSIGNAL);

	if(bytesSent < 0)
	{
		if (errno == EWOULDBLOCK)
		{
			return 0;
		}
		else if(errno == 0)
		{
			_TCPSocket->m_Status = TCPSocket_Status_Connected;
			return 0;
		}
		else if(errno == 32)
		{
			_TCPSocket->m_Status = TCPSocket_Status_Closed;
			return -2;
		}
		else 
		{
			printf("TCPSocket Error %i\r\n", errno);
			_TCPSocket->m_Status = TCPSocket_Status_Failed;
			return -1;
		}
	}
	else
	{
		_Buffer->m_BytesLeft -= bytesSent;
		_Buffer->m_ReadPtr += bytesSent;
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
