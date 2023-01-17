#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>

//Easy Networking in C (libcurl)
//Following https://www.youtube.com/watch?v=daA-KBKfJ_o
//By Jacob Sorber

typedef struct 
{
	int ID;
} Server;

size_t dataChunck(char* buffer, size_t itemsize, size_t nitems, void* context)
{
	size_t bytes = itemsize * nitems;
	Server* server = (Server*) context;

	printf("New chunk (%zu bytes)\n\r", bytes);
	printf("Context: %i\n\r", server->ID);

	return bytes;
}

// For reading user input for exiting the program without memorys leeks.
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#define False 1
#define True 0
#define Bool int

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

#include "Libs/Allocator.c"
#include "Libs/Server/DistroFiles_Server.c"

int kbhit(void);

int main(int argc, char* argv[])
{
	//Init the curl client
	CURL* curl = curl_easy_init();

	if(curl == NULL)
	{
		fprintf(stderr, "Init failed\n\r");
		return EXIT_FAILURE;
	}

	Server server;

	server.ID = 100;

	//Set options
	curl_easy_setopt(curl, CURLOPT_URL, "https://google.com");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, dataChunck);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &server);

	//perform our action
	CURLcode result = curl_easy_perform(curl);
	if(result != CURLE_OK)
		fprintf(stderr, "Download problem %s\n\r", curl_easy_strerror(result));

	

	curl_easy_cleanup(curl);
	return EXIT_SUCCESS;
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