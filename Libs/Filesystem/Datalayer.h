#ifndef Datalayer_h__
#define Datalayer_h__

struct T_Datalayer;
typedef struct T_Datalayer Datalayer;

#ifndef Datalayer_CRC
    #define Datalayer_CRC 0x10011
#endif

struct T_Datalayer
{
    Bool m_Allocated;
};

static inline UInt8 Datalayer_CRC(Datalayer* _Datalayer)
{
    UInt8 CRC = Datalayer_CRC << 1;

    printf("CRC: %s\n\r", BitHelper_GetString(CRC));

}

#endif // Datalayer_h__