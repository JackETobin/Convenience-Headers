#ifndef PLATFORM_TYPES_H
#define PLATFORM_TYPES_H

#include "platform_core.h"

#if defined(PLATFORM_WINDOWS)

typedef INT8                plat_int8;
typedef UINT8               plat_uint8;

typedef INT16               plat_int16;
typedef UINT16              plat_uint16;

typedef INT32               plat_int32;
typedef UINT32              plat_uint32;

typedef INT64               plat_int64;
typedef UINT64              plat_uint64;

#define stdcall WINAPI

#endif// PLATFORM_WINDOWS
#if defined(PLATFORM_LINUX)

typedef s8                  plat_int8;
typedef u8                  plat_uint8;

typedef s16                 plat_int16;
typedef u16                 plat_uint16;

typedef s32                 plat_int32;
typedef u32                 plat_uint32;

typedef s64                 plat_int64;
typedef u64                 plat_uint64;

#endif // PLATFORM_LINUX

#define null                ((void*)0)

#define true                (uint8)0X01
#define false               (uint8)0X00

typedef plat_int8           int8;
typedef plat_uint8          uint8;

typedef plat_int16          int16;
typedef plat_uint16         uint16;

typedef plat_int32          int32;
typedef plat_uint32         uint32;

typedef plat_int64          int64;
typedef plat_uint64         uint64;

typedef float               float32;
typedef double              float64;

typedef uint8               result;

#endif // PLATFORM_TYPES_H