#ifndef SystemConfig_h__
#define SystemConfig_h__

#define DistroFiles_Server_SyncTimeout 10000
#define DistroFiles_Server_SyncErrorTimeout (SEC) //TODO: One SEC might be too short.

#define DataLayer_CRC (0b11011)

#define DistroFiles_Checking_CheckError 50 //This is in %

#define DistroFiles_Service_BufferMax 1024
#define Payload_BufferSize 256
#define TCP_BufferSize 1024

#endif // SystemConfig_h__