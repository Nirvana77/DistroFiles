#include "Folder.h"

int Folder_Create(const char* _Path)
{
	int check;

	#ifdef __linux__
		check = mkdir(_Path, 0777);

		if(check == 0)
		{
			if(check == -1)
				check = 1; //Exists
			else 
				check = -1;
		}

	#elif _WIN32
		check = mkdir(_Path);
	#else
		printf("Not suported\n\r");
		return -1;
	#endif
	
	return check;
}