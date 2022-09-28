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
