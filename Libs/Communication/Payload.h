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
	int (*m_Receive)(void* _Context, Payload* _Message, Payload* _Replay);
	int (*m_Send)(void* _Context, Payload* _Paylode);
} Payload_FuncOut;

static inline void Payload_FuncOut_Set(Payload_FuncOut* _FuncOut, int (*_Receive)(void* _Context, Payload* _Message, Payload* _Replay), int (*_Send)(void* _Context, Payload* _Paylode), void* _Context)
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
	Payload_Type_Min = 0,

	Payload_Type_Broadcast = 0,
	Payload_Type_BroadcastRespons = 1,
	Payload_Type_ACK = 2,
	Payload_Type_UnSafe = 3,
	Payload_Type_Safe = 4,
	Payload_Type_Respons = 5,

	Payload_Type_Max = 5
} Payload_Type;

typedef enum
{
	Payload_Message_Type_None = 0,
	Payload_Message_Type_String = 1
} Payload_Message_Type;

typedef struct
{
	Payload_Message_Type m_Type;
	UInt16 m_Size;
	union
	{
		char m_Str[64]; //Make this a deff
	} m_Method;
	
	
} Payload_Message;

typedef enum
{
	Payload_Address_Type_IP = 0,
	Payload_Address_Type_MAC = 1
} Payload_Address_Type;

typedef struct
{
	Payload_Address_Type m_Type;
	union
	{

		//TODO: Add UUID
		// UInt8 UUID[UUID_DATA_SIZE];
		UInt8 IP[4];
		UInt8 MAC[6];
	} m_Address;
	
} Payload_Address;

struct T_Payload
{
	Bool m_Allocated;

	Payload_State m_State;

	UInt16 m_Size;
	UInt64 m_Time;

	Payload_Type m_Type;

	Payload_Message m_Message;

	Payload_Address m_Src;
	Payload_Address m_Des;

	Buffer m_Data;
};

int Payload_InitializePtr(Payload** _PayloadPtr);
int Payload_Initialize(Payload* _Payload);

int Payload_WriteCommunicator(Payload_Address* _Communicator, Buffer* _Buffer);
int Payload_ReadCommunicator(Payload_Address* _Communicator, Buffer* _Buffer);

int Payload_WriteMessage(Payload_Message* _Message, Buffer* _Buffer);
int Payload_ReadMessage(Payload_Message* _Message, Buffer* _Buffer);

void Payload_Copy(Payload* _Des, Payload* _Src);

static inline void Payload_Print(Payload* _Payload, const char* _Str)
{
	printf("Payload(%s): %lu\n\r", _Str, _Payload->m_Time);
	printf("State: %i\n\r", _Payload->m_State);
	printf("Type: %i\n\r", _Payload->m_Type);

	if(_Payload->m_Src.m_Type == Payload_Address_Type_IP)
		printf("SRC: %i.%i.%i.%i\n\r", _Payload->m_Src.m_Address.IP[0], _Payload->m_Src.m_Address.IP[1], _Payload->m_Src.m_Address.IP[2], _Payload->m_Src.m_Address.IP[3]);
	else
		printf("SRC: %x-%x-%x-%x-%x-%x\n\r", _Payload->m_Src.m_Address.MAC[0], _Payload->m_Src.m_Address.MAC[1], _Payload->m_Src.m_Address.MAC[2], _Payload->m_Src.m_Address.MAC[3], _Payload->m_Src.m_Address.MAC[4], _Payload->m_Src.m_Address.MAC[5]);
		

	if(_Payload->m_Des.m_Type == Payload_Address_Type_IP)
		printf("DES: %i.%i.%i.%i\n\r", _Payload->m_Des.m_Address.IP[0], _Payload->m_Des.m_Address.IP[1], _Payload->m_Des.m_Address.IP[2], _Payload->m_Des.m_Address.IP[3]);
	else
		printf("DES: %x-%x-%x-%x-%x-%x\n\r", _Payload->m_Des.m_Address.MAC[0], _Payload->m_Des.m_Address.MAC[1], _Payload->m_Des.m_Address.MAC[2], _Payload->m_Des.m_Address.MAC[3], _Payload->m_Des.m_Address.MAC[4], _Payload->m_Des.m_Address.MAC[5]);

	if(_Payload->m_Message.m_Type == Payload_Message_Type_None)
	{
		if(_Payload->m_Message.m_Type == Payload_Message_Type_String)
			printf("Method(%u): %s\n\r", _Payload->m_Message.m_Size, _Payload->m_Message.m_Method.m_Str);
	}

	printf("Data:\r\n");
	for (int i = 0; i < _Payload->m_Data.m_BytesLeft; i++)
		printf("%x%s", _Payload->m_Data.m_ReadPtr[i], i + 1< _Payload->m_Data.m_BytesLeft ? " " : "");
	printf("\n\r");
	
}

static inline void Payload_SetMessageType(Payload* _Payload, Payload_Message_Type _Type, void* _Value, int _Size)
{

	switch (_Type)
	{
		case Payload_Message_Type_String:
		{
			strncpy(_Payload->m_Message.m_Method.m_Str, (const char*)_Value, _Size);
			
			_Payload->m_Message.m_Type = _Type;
			_Payload->m_Message.m_Size = _Size;
		} break;
		case Payload_Message_Type_None:
		{
			_Payload->m_Message.m_Type = _Type;
			_Payload->m_Message.m_Size = 0;
			
		} break;
	}
}

void Payload_Dispose(Payload* _Payload);
#endif // Payload_h__