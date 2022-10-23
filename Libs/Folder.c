#include "Folder.h"

int Folder_Create(const char* _Path)
{
	int check;

	#ifdef __linux__
		check = mkdir(_Path, 0777);

		if(check != 0)
		{
			if(errno == EEXIST)
				check = 1;
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

int Folder_Hash(const char* _Path, unsigned char _Result[16])
{
	tinydir_dir dir;
	if(tinydir_open(&dir, _Path) != 0)
		return -1;

	MD5_CTX md5;
	MD5_Init(&md5);

	while (dir.has_next)
	{
		tinydir_file file;
		tinydir_readfile(&dir, &file);
		if(strcmp(file.name, ".") != 0 && strcmp(file.name, "..") != 0)
		{
			FILE* f = NULL;
			File_Open(file.path, "rb", &f);

			unsigned int totalSize = File_GetSize(f);
			
			char buffer[16];
			int i = 0;
			do
			{
				int length = strlen(file.name) - i;
				int bytesRead = strlen(strncpy(buffer, &file.name[i], length < 16 ? length : 16));
				MD5_Update(&md5, buffer, bytesRead);
				i += bytesRead;
			} while (i < strlen(file.name));
			
			
			if(totalSize != 0)
			{
				int bytesLeft = totalSize;
				while(bytesLeft > 0)
				{
					int bytesToRead = sizeof(buffer);
					if(bytesToRead > bytesLeft)
						bytesToRead = bytesLeft;
					
					int bytesRead = fread(buffer, 1, bytesToRead, f);
					if(bytesRead > 0)
					{
						MD5_Update(&md5, buffer, bytesRead);
						bytesLeft -= bytesRead;
					}
				}

			}
			

			if(f != NULL)
				File_Close(f);
			
		}
		tinydir_next(&dir);
	}

	tinydir_close(&dir);
	MD5_Final(_Result, &md5);
	return 0;
}