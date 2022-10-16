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

#include "Libs/File.c"
#include "Libs/Folder.c"
#include "Libs/Memory.c"
#include "Libs/String.c"
#include "Libs/LinkedList.c"
#include "Libs/StateMachine.c"
#include "Libs/Buffer.c"

#include "Libs/Allocator.c"
#include "Libs/BitHelper.c"

#include "Libs/Communication/Payload.c"
#include "Libs/Communication/DataLayer.c"
#include "Libs/Communication/TransportLayer.c"

#include "Libs/TCP/TCPSocket.c"
#include "Libs/TCP/TCPServer.c"
#include "Libs/TCP/TCPClient.c"

#include "Libs/Filesystem/Filesystem_Server.c"
#include "Libs/Filesystem/Filesystem_Client.c"
#include "Libs/Filesystem/Filesystem_Service.c"

int kbhit(void);

StateMachine g_StateMachine;

int main(int argc, char* argv[])
{
	#ifdef ALLOCATOR_DEBUG
		Allocator_Open("AllocatorDebug.txt");
	#endif
/* 
	if(argc > 1)
	{
		if(memcmp(argv[1], "daemon", 6) == 0)
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

		}
	} */

	int doExit = 1;
	StateMachine_Initialize(&g_StateMachine);
	
	Filesystem_Service* service = NULL;
	int success = Filesystem_Service_InitializePtr(&g_StateMachine, "Shared", &service);
	
	printf("Success: %i\r\n", success);

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

					case 'r':
					{
						
					} break;

					case 'w':
					{
						const char* str = "GET / HTTP/1.1\r\n\r\n";
						int size = strlen(str) + 1;
						/* Buffer buffer;
						Buffer_Initialize(&buffer, 64);

						printf("Client: %s\n\r", str);
						Buffer_WriteBuffer(&buffer, (UInt8*)str, strlen(str));

						TCPClient_Write(&service->m_Client->m_TCPClient, &buffer, strlen(str));
						Buffer_Dispose(&buffer);
						 */
						
						if(service != NULL)
							Filesystem_Client_SendMessage(service->m_Client, str, size);
						
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