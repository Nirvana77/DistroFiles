#include "Segmentation.h"

int Segmentation_InitializePtr(UInt8 _UUID[UUID_DATA_SIZE], Segmentation** _SegmentationPtr)
{
	Segmentation* _Segmentation = (Segmentation*)Allocator_Malloc(sizeof(Segmentation));
	if(_Segmentation == NULL)
		return -1;
	
	int success = Segmentation_Initialize(_Segmentation, _UUID);
	if(success != 0)
	{
		Allocator_Free(_Segmentation);
		return success;
	}
	
	_Segmentation->m_Allocated = True;
	
	*(_SegmentationPtr) = _Segmentation;
	return 0;
}

int Segmentation_Initialize(Segmentation* _Segmentation, UInt8 _UUID[UUID_DATA_SIZE])
{
	_Segmentation->m_Allocated = False;
	_Segmentation->m_IsComplite = False;
	_Segmentation->m_File = NULL;
	_Segmentation->m_Read = 0;
	_Segmentation->m_Write = 0;
	_Segmentation->m_BytesLeft = 0;

	char uuid[UUID_FULLSTRING_SIZE];
	uuid_ToString(_UUID, uuid);

	String_Initialize(&_Segmentation->m_Path, 32);

	String_Set(&_Segmentation->m_Path, uuid);
	String_Append(&_Segmentation->m_Path, ".seg", strlen(".seg"));

	if(File_Exist(_Segmentation->m_Path.m_Ptr) == True)
		File_Remove(_Segmentation->m_Path.m_Ptr);

	if(File_Open(_Segmentation->m_Path.m_Ptr, File_Mode_ReadApendBinary, &_Segmentation->m_File) != 0)
	{
		String_Dispose(&_Segmentation->m_Path);
		return -2;
	}
	
	
	return 0;
}

int Segmentation_Write(Segmentation* _Segmentation, unsigned char* _Buffer, int _Size)
{
	if(_Segmentation->m_IsComplite == True)
		return -1;

	File_SetPosition(_Segmentation->m_File, _Segmentation->m_Write);
	int written = File_WriteAll(_Segmentation->m_File, _Buffer, _Size);

	if(written < 0)
		return -2;

	if(written - _Size != 0)
		return -3;

	_Segmentation->m_Write += written;
	_Segmentation->m_BytesLeft += written;

	return written;
}

int Segmentation_Read(Segmentation* _Segmentation, unsigned char* _Buffer, int _Size)
{
	if(_Size > _Segmentation->m_BytesLeft)
		_Size = _Segmentation->m_BytesLeft;

	File_SetPosition(_Segmentation->m_File, _Segmentation->m_Read);
	int readed = File_Read(_Segmentation->m_File, _Buffer, _Size);

	if(readed < 0)
		return readed;

	if(readed - _Size != 0)
		return -1;

	_Segmentation->m_Read += readed;
	_Segmentation->m_BytesLeft -= readed;

	return readed;
}

int Segmentation_End(Segmentation* _Segmentation)
{
	UInt8 crc = 0;
	File_SetPosition(_Segmentation->m_File, 0);

	unsigned int fileSize = File_GetSize(_Segmentation->m_File);
	int bytesLeft = fileSize;
	if(bytesLeft < 0)
		return -1;
	else if(bytesLeft == 0)
		return 0;

	unsigned char ptr[Segmentation_FileBufferSize];
	int bytesRead = 0;
	int totalRead = 0;
	int bytesToRead = 0;
	int errorCheck = 0;

	while(bytesLeft > 0)
	{
		bytesToRead = bytesLeft;
		if(bytesToRead > Segmentation_FileBufferSize)
			bytesToRead = Segmentation_FileBufferSize;

		bytesRead = fread(ptr, 1, bytesToRead, _Segmentation->m_File);
		if(bytesRead < 0)
			return -2;

		else if(bytesRead == 0)
			errorCheck++;

		if(errorCheck == 10)
			return -3;

		DataLayer_GetCRC(ptr, bytesRead, &crc);

		bytesLeft -= bytesRead;
		totalRead += bytesRead;
	}
	
	fwrite(&crc, 1, 1, _Segmentation->m_File);
	
	_Segmentation->m_Write++;
	_Segmentation->m_BytesLeft++;

	_Segmentation->m_IsComplite = True;
	return 0;
}

void Segmentation_Dispose(Segmentation* _Segmentation)
{
	File_Close(_Segmentation->m_File);
	File_Remove(_Segmentation->m_Path.m_Ptr);

	String_Dispose(&_Segmentation->m_Path);

	if(_Segmentation->m_Allocated == True)
		Allocator_Free(_Segmentation);
	else
		memset(_Segmentation, 0, sizeof(Segmentation));

}