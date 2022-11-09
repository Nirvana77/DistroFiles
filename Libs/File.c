#include "File.h"

int File_Open(const char* _FilePath, File_Mode _Mode, FILE** _FilePtr)
{
	FILE* file = fopen(_FilePath, File_Mode_String[(int)_Mode]);
	if(file == NULL)
		return -1;

	*(_FilePtr) = file;
	return 0;
}

void File_SetPosition(FILE* _File, int _Position)
{
	fpos_t position;

	#if defined __linux__
		position.__pos = _Position;
	#else
		position = _Position;
	#endif

	fsetpos(_File, &position);
}

unsigned int File_GetSize(FILE* _File)
{
	File_SetPosition(_File, 0);

	fseek(_File, 0, SEEK_END);
	unsigned int size = ftell(_File);

	File_SetPosition(_File, 0);

	return size;
}

void File_Close(FILE* _File)
{
	fflush(_File);
	fclose(_File);
}

int File_Hash(FILE* _File, unsigned char _Result[16])
{
	unsigned int totalSize = File_GetSize(_File);
	if(totalSize == 0)
		return -1;

	MD5_CTX md5;
	MD5_Init(&md5);

	int bytesLeft = totalSize;
	char buffer[8];
	
	while(bytesLeft > 0)
	{
		int bytesToRead = sizeof(buffer);
		if(bytesToRead > bytesLeft)
			bytesToRead = bytesLeft;
		
		int bytesRead = fread(buffer, 1, bytesToRead, _File);
		if(bytesRead > 0)
		{
			MD5_Update(&md5, buffer, bytesRead);
			bytesLeft -= bytesRead;
		}
	}

	MD5_Final(_Result, &md5);
	return 0;
}


int File_Read(FILE* _File, unsigned char* _Buffer, int _Size)
{
	unsigned char* ptr = _Buffer;
	int bytesRead;
	while(_Size > 0)
	{
		bytesRead = fread(ptr, 1, _Size, _File);
		if(bytesRead < 0)
			return -1;

		ptr += bytesRead;
		_Size -= bytesRead;
	}

	return 0;
}

int File_ReadAll(FILE* _File, unsigned char* _Buffer, int _BufferSize)
{
	if(_Buffer == NULL)
		return 1;

	File_SetPosition(_File, 0);

	unsigned int fileSize = File_GetSize(_File);
	int bytesLeft = fileSize;
	if(bytesLeft < 0)
		return -1;
	else if(bytesLeft == 0)
		return 0;

	unsigned char* ptr = _Buffer;
	int bytesRead = 0;
	int totalRead = 0;
	int bytesToRead = 0;
	int errorCheck = 0;

	while(bytesLeft > 0 && totalRead < _BufferSize)
	{
		bytesToRead = bytesLeft;
		if(bytesToRead > _BufferSize - totalRead)
			bytesToRead = _BufferSize - totalRead;

		bytesRead = fread(ptr, 1, bytesToRead, _File);
		if(bytesRead < 0)
			return -2;

		else if(bytesRead == 0)
			errorCheck++;

		if(errorCheck == 10)
			return -3;

		ptr += bytesRead;
		bytesLeft -= bytesRead;
		totalRead += bytesRead;
	}

	return totalRead;
}

int File_WriteAll(FILE* _File, const unsigned char* _Data, int _DataSize)
{
	int totalWritten = 0;
	const unsigned char* ptr = _Data;
	while(_DataSize > 0)
	{
		int bytesWritten = fwrite(ptr, 1, _DataSize, _File);
		if(bytesWritten > 0)
		{
			_DataSize -= bytesWritten;
			ptr += bytesWritten;
			totalWritten += bytesWritten;
		}
		else
		{
			return -1;
		}
	}
	return totalWritten;
}


int File_Move(const char* _Source, const char* _Destination)
{
	int renameSuccess = rename(_Source, _Destination);

	if(renameSuccess != 0)
	{
		if(File_Remove(_Destination) != 0)
			return -1;

		if(File_Move(_Source, _Destination) != 0)
			return -2;
	}

	return 0;
}

int File_Copy(const char* _Source, const char* _Destination)
{
	FILE* sourceFile = fopen(_Source, "r");
	if(sourceFile == NULL)
		return -1;

	remove(_Destination);

	FILE* destinationFile = fopen(_Destination, "w");
	if(destinationFile == NULL)
		return -2;

	int success = 0;

	unsigned int size = File_GetSize(sourceFile);
	if(size > 0)
	{
		unsigned int bytesLeft = size;
		int bytesRead;
		int bytesWritten;
		char buffer[8];
		char* ptr = buffer;
		while(bytesLeft > 0)
		{
			ptr = buffer;
			bytesRead = fread(ptr, 1, bytesLeft < sizeof(buffer) ? bytesLeft : sizeof(buffer), sourceFile);
			if(bytesRead > 0)
			{
				bytesLeft -= bytesRead;

				while(bytesRead > 0)
				{
					bytesWritten = fwrite(ptr, 1, bytesRead, destinationFile);
					if(bytesWritten > 0)
					{
						bytesRead -= bytesWritten;
						ptr += bytesWritten;
					}
					else
					{
						printf("Write to file \"%s\" failed! (%i)", _Destination, bytesWritten);
						success = -3;
						break;
					}
				}

				if(success != 0)
					break;

			}
			else
			{
				printf("Read from file \"%s\" failed! (%i)", _Source, bytesRead);
				success = -4;
				break;
			}
		}

	}

	//fcopy(sourceFile, destinationFile);

	/*
	int chr = fgetc(sourceFile);
	while(chr != EOF)
	{
		fputc(chr, destinationFile);
		chr = fgetc(sourceFile);
	}
	*/

	fclose(sourceFile);
	fclose(destinationFile);

	return success;
}

int File_Remove(const char* _Source)
{
	if(remove(_Source) == 0)
		return 0;

	return -1;
}
