#ifndef SystemConfig_h__
#define SystemConfig_h__

#define Filesystem_Server_SyncTimeout 10000
#define Filesystem_Server_SyncErrorTimeout (SEC)

#define DataLayer_CRC (0b11011)

#define Filesystem_Checking_CheckError 50 //This is in %

#define Filesystem_Service_BufferMax 1024
#define Payload_BufferSize 256
#define TCP_BufferSize 1024

#endif // SystemConfig_h__