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
	Payload_State_Init = 0,
	Payload_State_Sented = 1,
	Payload_State_Sending = 2,
	Payload_State_Resived = 3,
	Payload_State_Removed = 4,
	Payload_State_Destroyed = 5,
	Payload_State_Failed = 6
} Payload_State;

typedef enum
{
	Payload_MessageType_Min = 0,

	Payload_MessageType_Broadcast = 0,
	Payload_MessageType_BroadcastRespons = 1,
	Payload_MessageType_ACK = 2,
	Payload_MessageType_UnSafe = 3,
	Payload_MessageType_Safe = 4,
	Payload_MessageType_Respons = 5,

	Payload_MessageType_Max = 5
} Payload_MessageType;

typedef enum
{
	Payload_Communicator_Type_IP = 0,
	Payload_Communicator_Type_MAC = 1
} Payload_Type;

typedef struct
{
	Payload_Type m_Type;
	union
	{

		//TODO: Add UUID
		// UInt8 UUID[UUID_DATA_SIZE];
		UInt8 IP[4];
		UInt8 MAC[6];
	} m_Address;
	
} Payload_Communicator;

struct T_Payload
{
	Bool m_Allocated;

	Payload_State m_State;

	UInt16 m_Size;
	UInt64 m_Time;

	Payload_MessageType m_Type;

	Payload_Communicator m_Src;
	Payload_Communicator m_Des;

	Buffer m_Data;
};

int Payload_InitializePtr(Payload** _PayloadPtr);
int Payload_Initialize(Payload* _Payload);

int Payload_WriteCommunicator(Payload_Communicator* _Communicator, Buffer* _Buffer);
int Payload_ReadCommunicator(Payload_Communicator* _Communicator, Buffer* _Buffer);

static inline void PayloadFilCommunicator(Payload_Communicator* _Des, Payload_Communicator* _Src)
{
	_Des->m_Type = _Src->m_Type;
	switch (_Src->m_Type)
	{
		case Payload_Communicator_Type_IP:
		{
			_Des->m_Address.IP[0] = _Src->m_Address.IP[0];
			_Des->m_Address.IP[1] = _Src->m_Address.IP[1];
			_Des->m_Address.IP[2] = _Src->m_Address.IP[2];
			_Des->m_Address.IP[3] = _Src->m_Address.IP[3];
		} break;
		case Payload_Communicator_Type_MAC:
		{
			_Des->m_Address.MAC[0] = _Src->m_Address.MAC[0];
			_Des->m_Address.MAC[1] = _Src->m_Address.MAC[1];
			_Des->m_Address.MAC[2] = _Src->m_Address.MAC[2];
			_Des->m_Address.MAC[3] = _Src->m_Address.MAC[3];
			_Des->m_Address.MAC[4] = _Src->m_Address.MAC[4];
			_Des->m_Address.MAC[5] = _Src->m_Address.MAC[5];

		} break;
	}
}

static inline void Payload_Copy(Payload* _Des, Payload* _Src)
{
	_Des->m_Size = _Src->m_Size;
	_Des->m_Time = _Src->m_Time;
	_Des->m_Type = _Src->m_Type;
	_Des->m_State = _Src->m_State;

	PayloadFilCommunicator(&_Des->m_Des, &_Src->m_Des);
	PayloadFilCommunicator(&_Des->m_Src, &_Src->m_Src);

	Buffer_Copy(&_Des->m_Data, &_Src->m_Data, 0);
}

static inline void Payload_Print(Payload* _Payload, const char* _Str)
{
	printf("Payload(%s): %lu\n\r", _Str, _Payload->m_Time);
	printf("State: %i\n\r", _Payload->m_State);
	printf("Type: %i\n\r", _Payload->m_Type);

	printf("SRC: %i.%i.%i.%i\n\r", _Payload->m_Src.m_Address.IP[0], _Payload->m_Src.m_Address.IP[1], _Payload->m_Src.m_Address.IP[2], _Payload->m_Src.m_Address.IP[3]);
	printf("DES: %i.%i.%i.%i\n\r", _Payload->m_Des.m_Address.IP[0], _Payload->m_Des.m_Address.IP[1], _Payload->m_Des.m_Address.IP[2], _Payload->m_Des.m_Address.IP[3]);

	printf("Data:\r\n");
	for (int i = 0; i < _Payload->m_Data.m_BytesLeft; i++)
		printf("%x%s", _Payload->m_Data.m_ReadPtr[i], i + 1< _Payload->m_Data.m_BytesLeft ? " " : "");
	printf("\n\r");
	
}

void Payload_Dispose(Payload* _Payload);
#endif // Payload_h__