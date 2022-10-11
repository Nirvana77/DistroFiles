#ifndef Payload_h__
#define Payload_h__

struct T_Payload;
typedef struct T_Payload Payload;


#ifndef Payload_BufferSize
	#define Payload_BufferSize 256
#endif
typedef struct
{
	void* m_Context;
	int (*m_Receive)(void* _Context, Payload* _Paylode);
	int (*m_Send)(void* _Context, Payload* _Paylode);
} Payload_FuncOut;

static inline void Payload_FuncOut_Set(Payload_FuncOut* _FuncOut, int (*_Receive)(void* _Context, Payload* _Paylode), int (*_Send)(void* _Context, Payload* _Paylode), void* _Context)
{
	_FuncOut->m_Context = _Context;
	_FuncOut->m_Receive = _Receive;
	_FuncOut->m_Send = _Send;
}

static inline void Payload_FuncOut_Clear(Payload_FuncOut* _FuncOut)
{
	memset(_FuncOut, 0, sizeof(Payload_FuncOut));
}

typedef enum
{
	Payload_Type_UI8 = 0,
	Payload_Type_UI16 = 1,
	Payload_Type_UI32 = 2,
	Payload_Type_UI64 = 3,
	Payload_Type_BUFFER = 4,
	Payload_Type_IP = 5,
	Payload_Type_MAC = 6
} Payload_Type;

struct T_Payload
{
	Bool m_Allocated;
	Bool m_Dynamic;

	UInt16 m_Size;
	UInt64 m_Time;

	Payload_Type m_Type;
	union 
	{
		UInt8 UI8;
		UInt16 UI16;
		UInt32 UI32;
		UInt64 UI64;
		UInt8 BUFFER[Payload_BufferSize];

		//TODO: Add UUID
		// UInt8 UUID[UUID_DATA_SIZE];
		UInt8 IP[4];
		UInt8 MAC[6];
	} m_Data;
};

int Payload_InitializePtr(Payload** _PayloadPtr);
int Payload_Initialize(Payload* _Payload);


void Payload_Dispose(Payload* _Payload);
#endif // Payload_h__