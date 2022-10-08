#ifndef Types_h__
#define Types_h__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define False 1
#define True 0
#define Bool int

#ifdef __linux__
    #include <inttypes.h>
    #include <stdint.h>

    typedef int8_t Int8;
    typedef uint8_t UInt8;
    typedef unsigned char Byte;
    typedef int16_t Int16;
    typedef uint16_t UInt16;
    typedef int32_t Int32;
    typedef uint32_t UInt32;
    typedef int64_t Int64;
    typedef uint64_t UInt64;
    typedef float Float;
    typedef double Double;
    
#elif _WIN32

#else

#endif

#endif // Types_h__