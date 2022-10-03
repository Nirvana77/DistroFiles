#ifndef Memory_h__
#define Memory_h__

struct T_Memory;
typedef struct T_Memory Memory;

struct T_Memory
{
	
};

int Memory_UInt32ToBuffer(u_int32_t* _Src, unsigned char* _Pointer);
int Memory_UInt16ToBuffer(u_int16_t* _Src, unsigned char* _Pointer);
int Memory_UInt8ToBuffer(u_int8_t* _Src, unsigned char* _Pointer);

int Memory_ParseUInt32(const void* _Pointer, u_int32_t* _Dest);
int Memory_ParseUInt16(const void* _Pointer, u_int16_t* _Dest);
int Memory_ParseUInt8(const void* _Pointer, u_int8_t* _Dest);

#endif // Memory_h__