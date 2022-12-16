#ifndef Segmentation_h__
#define Segmentation_h__

struct T_Segmentation;
typedef struct T_Segmentation Segmentation;

#include "../uuid.h"
#include "../File.h"
#include "../String.h"

#define Segmentation_FileBufferSize 16

struct T_Segmentation
{
	Bool m_Allocated;
	FILE* m_File;
	String m_Path;

	int m_Read;
	int m_Write;
	int m_BytesLeft;
	Bool m_IsComplite;
};

int Segmentation_InitializePtr(UInt8 _UUID[UUID_DATA_SIZE], Segmentation** _SegmentationPtr);
int Segmentation_Initialize(Segmentation* _Segmentation, UInt8 _UUID[UUID_DATA_SIZE]);

int Segmentation_Write(Segmentation* _Segmentation, unsigned char* _Buffer, int _Size);
int Segmentation_Read(Segmentation* _Segmentation, unsigned char* _Buffer, int _Size);
int Segmentation_End(Segmentation* _Segmentation);

void Segmentation_Dispose(Segmentation* _Segmentation);
#endif // Segmentation_h__