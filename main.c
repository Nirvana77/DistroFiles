#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// For reading user input for exiting the program without memorys leeks.
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef COMPILEDEBUG
	#define ALLOCATOR_DEBUG
	#define ALLOCATOR_DEBUG_BORDERCHECK 6
	#define ALLOCATOR_RUN_AFTERCHECK
#endif

// #include "Libs/Portability.c"

#include "Libs/Hash/md5.c"
#include "Libs/File.c"
#include "Libs/Folder.c"
#include "Libs/Memory.c"
#include "Libs/String.c"
#include "Libs/LinkedList.c"
#include "Libs/StateMachine.c"
#include "Libs/Buffer.c"

#include "Libs/Allocator.c"
#include "Libs/BitHelper.c"
#include "Libs/uuid.c"

#include "Libs/Communication/Payload.c"
#include "Libs/Communication/DataLayer.c"
#include "Libs/Communication/NetworkLayer.c"
#include "Libs/Communication/TransportLayer.c"

#include "Libs/TCP/TCPSocket.c"
#include "Libs/TCP/TCPServer.c"
#include "Libs/TCP/TCPClient.c"

#include "Libs/Filesystem/Filesystem_Server.c"
#include "Libs/Filesystem/Filesystem_Client.c"
#include "Libs/Filesystem/Filesystem_Service.c"

void printHash(unsigned char result[16]);
int kbhit(void);

StateMachine g_StateMachine;


//TODO: #54 LinkedList is not disposed correctly
int main(int argc, char* argv[])
{
	#ifdef ALLOCATOR_DEBUG
		Allocator_Open("AllocatorDebug.txt");
	#endif
	char* path = "Shared";
	if(argc > 1)
	{
		path = argv[1];
		if(argc > 2)
		{
			/*
			if(memcmp(argv[2], "daemon", 6) == 0)
			{


				printf("Starting daemon...\r\n");
				pid_t pid, sid;
			
				pid = fork();
				if (pid < 0) {
					printf("Failed to fork!\r\n");
					exit(EXIT_FAILURE);
				}

				if (pid > 0) {
					printf("Stopping parent.\r\n");
					exit(EXIT_SUCCESS);
				}

				umask(0);
				sid = setsid();
				if (sid < 0) {
					printf("Failed to create SID for child process!\r\n");
					exit(EXIT_FAILURE);
				}
				
			
				signal(SIGUSR1, SIG_IGN);
				signal(SIGALRM, SIG_IGN);

				signal(SIGCHLD, SIG_IGN);   // A child process dies 
				signal(SIGTSTP, SIG_IGN);   // Various TTY signals
				signal(SIGTTOU, SIG_IGN);
				signal(SIGTTIN, SIG_IGN);
				signal(SIGHUP, SIG_IGN);    // Ignore hangup signal
				signal(SIGTERM, SIG_DFL);   // Die on SIGTERM
				

				freopen("/dev/null", "a", stdout);
				freopen("/dev/null", "a", stderr);
				freopen("Filesystem.inp", "r", stdin);

			}*/

		}
		
	}

	int doExit = 1;
	StateMachine_Initialize(&g_StateMachine);
	
	Folder_Remove("Shared/root");
	Folder_Remove("Shared/temp");
	File_Remove("payload_dump.txt");
	
	Filesystem_Service* service = NULL;
	int success = Filesystem_Service_InitializePtr(&g_StateMachine, path, &service);
	
	printf("Success: %i\r\n", success);
	if(success == 0)
	{
		printf("Port: %u\n\r", (unsigned int)ntohs(service->m_Server->m_TCPServer.m_ServerAddr.sin_port));
	}

	struct timespec tim, tim2;
	tim.tv_sec = 0;
	tim.tv_nsec = 0;
	while(doExit == 1)
	{
		StateMachine_Work(&g_StateMachine);
		
		if(kbhit())
		{
			UInt8 chr = getchar();
			if(chr == 'q')
				doExit = 0;

			else if(chr == 'c')
				system("clear");
			
			else
			{
				switch (chr)
				{
					case 'i':
					{
						UInt8 add[4];
						UInt8 mac[6];
						add[0] = 12;
						GetIP(add);
						GetMAC(mac);

						printf("IP: ");
						for (int i = 0; i < 4; i++)
						{
							printf("%d%s", add[i], i + 1 < 4 ? "." : "");
						}
						printf("\n\r");

						printf("MAC: ");
						for (int i = 0; i < 6; i++)
						{
							printf("%x%s", mac[i], i + 1 < 6 ? "." : "");
						}
						printf("\n\r");

					} break;

					case 'h':
					{
						FILE* f = NULL;
						File_Open("Shared/root/test.txt", File_Mode_ReadBinary, &f);


						if(f != NULL)
						{
							unsigned char hash[16];
							memset(hash, 0, 16);

							File_Hash(f, hash);
							printf("Hash:\t");
							printHash(hash);

							File_Close(f);
						}
						else
						{
							printf("No file\n\r");
						}
					} break;

					case 'f':
					{
						unsigned char hash[16];
						memset(hash, 0, 16);
						Folder_Hash("Shared/root/", hash);
						printf("Hash:\t");
						printHash(hash);

					} break;

					case 's':
					{
						Payload* message = NULL;
						char* path = "root";

						int size = 2 + strlen(path) + 16;

						if(TransportLayer_CreateMessage(&service->m_Server->m_TransportLayer, Payload_Type_Broadcast, size, 1000, &message) == 0)
						{
							Buffer_WriteUInt16(&message->m_Data, strlen(path));
							Buffer_WriteBuffer(&message->m_Data, (unsigned char*)path, strlen(path));

							unsigned char hash[16];
							Folder_Hash(service->m_Server->m_FilesytemPath.m_Ptr, hash);
							Buffer_WriteBuffer(&message->m_Data, hash, 16);

							Payload_SetMessageType(message, Payload_Message_Type_String, "Sync", strlen("Sync"));
						}
					} break;

					case 'w':
					{

						
						if(service != NULL)
						{
							String fullPath;
							const char* path = "test.txt";
							String_Initialize(&fullPath, 8);
							FILE* f = NULL;

							String_Set(&fullPath, service->m_Server->m_FilesytemPath.m_Ptr);

							if(String_EndsWith(&fullPath, "/") == False)
								String_Append(&fullPath, "/", 1);
							String_Append(&fullPath, path, strlen(path));

							printf("Full path: %s\n\r", fullPath.m_Ptr);

							File_Open(fullPath.m_Ptr, File_Mode_ReadBinary, &f);

							int size = 1 + 2 + strlen(path) + 2 + File_GetSize(f);

							Payload* message = NULL;
							if(TransportLayer_CreateMessage(&service->m_Server->m_TransportLayer, Payload_Type_Safe, size, 1000, &message) == 0)
							{
								Buffer_WriteUInt8(&message->m_Data, True);
								Buffer_WriteUInt16(&message->m_Data, (UInt16)(strlen(path)));
								Buffer_WriteBuffer(&message->m_Data, (UInt8*)path, strlen(path));
								
								Payload_SetMessageType(message, Payload_Message_Type_String, "Write", strlen("Write"));

								Buffer_WriteUInt16(&message->m_Data, (UInt16)File_GetSize(f));
								Buffer_ReadFromFile(&message->m_Data, f);
							}

							File_Close(f);
							String_Dispose(&fullPath);
						}
					} break;

					case 'd':
					{
						Payload message;

						Payload_Initialize(&message, NULL);

						SystemMonotonicMS(&message.m_Time);
						
						Payload_SetMessageType(&message, Payload_Message_Type_String, "Test", strlen("Test"));

						message.m_Src.m_Type = Payload_Address_Type_IP;
						GetIP(message.m_Src.m_Address.IP);

						message.m_Des.m_Type = Payload_Address_Type_MAC;
						message.m_Des.m_Address.MAC[0] = 0xe3;
						message.m_Des.m_Address.MAC[1] = 0x3;
						message.m_Des.m_Address.MAC[2] = 0xf5;
						message.m_Des.m_Address.MAC[3] = 0xf0;
						message.m_Des.m_Address.MAC[4] = 0x23;
						message.m_Des.m_Address.MAC[5] = 0x2;

						message.m_Type = Payload_Type_ACK;
						const char* str = "Hellow, world";

						message.m_State = Payload_State_Sending;
						message.m_Size = strlen(str);
						Buffer_WriteBuffer(&message.m_Data, (unsigned char*)str, strlen(str));

						void* ptr = &message;
						int length = sizeof(message);
						unsigned char data[sizeof(message)];
						memcpy(data, ptr, length);

						printf("Payload(%u): \r\n", length);
						for (int i = 0; i < length; i++)
							printf("%x ", data[i]);
						printf("\r\n");
						
						length = sizeof(Payload_Address);
						memcpy(data, &message.m_Des, length);
						printf("Payload_Address(%u): \r\n", length);
						for (int i = 0; i < length; i++)
							printf("%x ", data[i]);
						printf("\r\n");
						memcpy(data, &message.m_Src, length);
						printf("Payload_Address(%u): \r\n", length);
						for (int i = 0; i < length; i++)
							printf("%x ", data[i]);
						printf("\r\n");
						
						Payload networkMessage;
						Payload_Initialize(&networkMessage, NULL);
						Payload_Copy(&networkMessage, &message);
						NetworkLayer_PayloadBuilder(NULL, &networkMessage);
						
						printf("NetworkLayerPayload size: %u\r\n", networkMessage.m_Data.m_BytesLeft);
						Payload_Print(&networkMessage, "NetworkLayerPayload", True);

						Payload_Dispose(&networkMessage);
						Payload_Dispose(&message);
					} break;

					case 'b':
					{
						printf("CRC: ");
						for (int i = 0; i < 0xff + 1; i++)
						{
							UInt8 crc = 0;
							DataLayer_GetCRC((unsigned char*)&i, 1, &crc);
							printf("%i ", crc);
							
						}
						printf("\r\n");
						
					} break;
				
					default:
					{
						printf(">%c", chr);

					}break;
				}
			}
				

		}

		nanosleep(&tim, &tim2);
	}

	if(service != NULL)
		Filesystem_Service_Dispose(service);

	StateMachine_Dispose(&g_StateMachine);

	#ifdef ALLOCATOR_DEBUG
		Allocator_Close();
	#endif
	
	return 0;
}

void printHash(unsigned char result[16])
{
	for(int i = 0; i < 16; i++)
		printf("%x", result[i]);
	printf("\n\r");
}

int kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;
 
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
  ch = getchar();
 
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);
 
  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }
 
  return 0;
}