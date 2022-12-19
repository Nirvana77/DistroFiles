#include "Folder.h"

int Folder_InternalHash(const char* _Path, MD5_CTX* _MD5);

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
	MD5_CTX md5;
	MD5_Init(&md5);

	Folder_InternalHash(_Path, &md5);

	MD5_Final(_Result, &md5);
	return 0;
}

int Folder_InternalHash(const char* _Path, MD5_CTX* _MD5)
{

	tinydir_dir dir;
	if(tinydir_open(&dir, _Path) != 0)
		return -1;

	while (dir.has_next)
	{
		tinydir_file file;
		tinydir_readfile(&dir, &file);
		if(strcmp(file.name, ".") != 0 && strcmp(file.name, "..") != 0)
		{
			if(file.is_dir)
			{
				char des[strlen(_Path) + 1 + strlen(file.name)];
				strcpy(des, _Path);
				des[strlen(_Path)] = '/';
				strcpy(&des[strlen(_Path) + 1], file.name);
				
				//Folder_InternalHash(des, _MD5);
			}
			else
			{
				FILE* f = NULL;
				File_Open(file.path, File_Mode_ReadBinary, &f);

				unsigned int totalSize = File_GetSize(f);
				
				char buffer[16];
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
							MD5_Update(_MD5, buffer, bytesRead);
							bytesLeft -= bytesRead;
						}
					}

				}
				

				if(f != NULL)
					File_Close(f);
			}
			
		}
		tinydir_next(&dir);
	}

	tinydir_close(&dir);
	return 0;
}

Bool Folder_Exist(const char* _Path)
{
	int success = Folder_Create(_Path);
	if(success == 0)
	{
		Folder_Remove(_Path);
		return False;
	}
	else if(success == 1)
	{
		return True;
	}

	return False;
}

int Folder_Remove(const char* _Path)
{
	tinydir_dir dir;
	if(tinydir_open(&dir, _Path) != 0)
		return -1;

	while (dir.has_next)
	{
		tinydir_file file;
		tinydir_readfile(&dir, &file);
		if(strcmp(file.name, ".") != 0 && strcmp(file.name, "..") != 0)
		{
			if(file.is_dir)
				Folder_Remove(file.path);
			else
				File_Remove(file.path);
			
		}
		tinydir_next(&dir);
	}

	tinydir_close(&dir);

	if(remove(_Path) == 0)
		return 0;

	return -2;
}

int Folder_Move(char* _Source, char* _Destination)
{
	if(Folder_Create(_Destination) < 0)
		return -1;

	tinydir_dir dir;
	if(tinydir_open(&dir, _Source) != 0)
		return -2;
		
	while (dir.has_next)
	{
		tinydir_file file;
		tinydir_readfile(&dir, &file);
		if(strcmp(file.name, ".") != 0 && strcmp(file.name, "..") != 0)
		{
			char des[strlen(_Destination) + 1 + strlen(file.name)];
			strcpy(des, _Destination);
			des[strlen(_Destination)] = '/';
			strcpy(&des[strlen(_Destination) + 1], file.name);
			if(file.is_dir)
				Folder_Move(file.path, des);
			
			else
				File_Move(file.path, des);
					
		}
		tinydir_next(&dir);
	}

	tinydir_close(&dir);

	if(remove(_Source) == 0)
		return 0;
	
	return -3;
}