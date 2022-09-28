#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>

//Easy Networking in C (libcurl)
//Following https://www.youtube.com/watch?v=daA-KBKfJ_o
//By Jacob Sorber

size_t dataChunck(char* buffer, size_t itemsize, size_t nitems, void* context)
{
	size_t bytes = itemsize * nitems;

	printf("New chunk (%zu bytes)\n\r", bytes);


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

	//Set options
	curl_easy_setopt(curl, CURLOPT_URL, "https://google.com");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, dataChunck);

	//perform our action
	CURLcode result = curl_easy_perform(curl);
	if(result != CURLE_OK)
		fprintf(stderr, "Download problem %s\n\r", curl_easy_strerror(result));

	

	curl_easy_cleanup(curl);
	return EXIT_SUCCESS;
}
