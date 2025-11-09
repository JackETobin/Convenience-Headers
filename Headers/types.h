#ifndef TYPES_H
#define TYPES_H

#if defined(PLATFORM_TYPES_H)
    #include "Platform_Headers/platform_types.h"
#else

    typedef char                int8;
    typedef unsigned char       uint8;

    typedef short               int16;
    typedef unsigned short      uint16;

    typedef int                 int32;
    typedef unsigned int        uint32;

    typedef long long           int64;
    typedef unsigned long long  uint64;

    typedef float               float32;
    typedef double              float64;

    typedef uint8               result;
#endif // PLATFORM_TYPES_H
    
#endif // TYPES_H