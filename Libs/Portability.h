#ifndef Portability_h__
#define Portability_h__

#include "Types.h"
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <time.h>

static inline UInt64 SystemMonotonicMS(UInt64* _ResultPtr)
{
	long ms; 
	time_t s; 

	struct timespec spec;
	UInt64 result = 0;
	clock_gettime(CLOCK_REALTIME, &spec);
	s  = spec.tv_sec;
	ms = (spec.tv_nsec / 1.0e6);

	result = s;
	result *= 1000;
	result += ms;

    *(_ResultPtr) = result;
	return result;
}

#endif // Portability_h__