#ifndef MEM_RESERVE_H
#define MEM_RESERVE_H

#if !defined(MEM_HEADERS_H) || !defined(PLATFORM_H)
    #define null                ((void*)0)
    #define true                (uint8)0X01
    #define false               (uint8)0X00

    typedef unsigned char       uint8;
    typedef unsigned short      uint16;
    typedef unsigned int        uint32;
    typedef unsigned long long  uint64;

    typedef double              float64;
    typedef uint8               result;
#endif // MEM_TYPES

#define RES_SUCCESS (result)0X04

#if defined(MODE_DEBUG)
    #if !defined(MEM_HEADERS_H)
        #include "../log.h"
        #define DBG(func) _Debug_Catch(func, #func, line, file)
    #endif // MEM_HEADERS_H
#else
    #if !defined(MEM_HEADERS_H)
        #define DBG(func) func
    #endif // MEM_HEADERS_H
#endif // MODE_DEBUG

result
_Res_Create()
{
    return RES_SUCCESS;
};

result
_Res_Erase()
{
    return RES_SUCCESS;
};

result
_Res_Write()
{
    return RES_SUCCESS;
};

result
_Res_RetrieveRawPtr()
{
    return RES_SUCCESS;
};

uint64
_Res_Size()
{
    return 0;
};

#endif // MEM_RESERVE_H