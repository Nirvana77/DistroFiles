#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main()
{
	char* ip = "127.0.0.1";
	int port = 5566;

	int sock;
	struct sockaddr_in addr;
	socklen_t addr_size;
	char buffer[1024];
	int n;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0)
	{
		printf("Socket error\n\r");
		return EXIT_FAILURE;
	}
	printf("TCP server socket created.\n\r");
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = port;
	addr.sin_addr.s_addr = inet_addr(ip);

	connect(sock, (struct sockaddr*)&addr, sizeof(addr));
	printf("Connected to the server\n\r");

	bzero(buffer, sizeof(buffer));
	strcpy(buffer, "Hellow, server!");

	printf("Client: %s\n\r", buffer);

	send(sock, buffer, strlen(buffer), 0);

	bzero(buffer, sizeof(buffer));
	recv(sock, buffer, sizeof(buffer), 0);

	printf("Server: %s\n\r", buffer);

	close(sock);
	printf("Disconnected from the server\n\r");

	return 0;
}