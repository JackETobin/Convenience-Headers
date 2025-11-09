#ifndef PLATFORM_ALLOC_H
#define PLATFORM_ALLOC_H

#include "platform_types.h"

#if defined(PLATFORM_WINDOWS)
    typedef DWORD l_page;
    
    l_page
    QueryPageSize(void)
    {
        SYSTEM_INFO sysInfo = { 0 };
        GetSystemInfo(&sysInfo);
        return sysInfo.dwPageSize;
    }
    #define PageSize() QueryPageSize();

    #define SysAlloc(pStart, size) VirtualAlloc(pStart, size,               \
                                                MEM_COMMIT | MEM_RESERVE,   \
                                                PAGE_READWRITE)
    #define SysFree(pBlock, size) VirtualFree(pBlock, size,                 \
                                              MEM_RELEASE |                 \
                                              MEM_COALESCE_PLACEHOLDERS)    
#endif // PLATFORM_WINDOWS
#if defined(PLATFORM_LINUX)
    typedef int l_page;
    #define PageSize() getpagesize()
    #define SysAlloc(pStart, size) mmap(pStart, size,                       \
                                        PROT_READ | PROT_WRITE,             \
                                        MAP_PRIVATE | MAP_ANONYMOUS,        \
                                        0, 0
    #define SysFree(pBlock, size) munmap(pBlock, size)
#endif // PLATFORM_LINUX

#define PLAT_SUCCESS        (result)0X00 // Platform success.
#define PLAT_NOALLOC        (result)0X2A // Platform unable to allocate memory.
#define PLAT_NEGEXT         (result)0X2B // Platform realloc cannot shrink an allocation.
#define PLAT_BUFFLIM        (result)0X2C // Platform virtual alloc extension cap reached.

#if     defined(PLAT_ALLOC_BUFFER_256GB)
        #undef PLAT_ALLOC_BUFFER
        #define PLAT_ALLOC_BUFFER 0X4000000000llu
#elif   defined(PLAT_ALLOC_BUFFER_128GB)
        #undef PLAT_ALLOC_BUFFER
        #define PLAT_ALLOC_BUFFER 0X2000000000llu

#elif   defined(PLAT_ALLOC_BUFFER_64GB)
        #undef PLAT_ALLOC_BUFFER
        #define PLAT_ALLOC_BUFFER 0X1000000000llu
#else
        #undef PLAT_ALLOC_BUFFER
        #define PLAT_ALLOC_BUFFER 0X800000000llu
#endif // PLAT_ALLOC_BUFFER_XXGB

#define PLAT_START_ADDR (uint8*)0X300000000

static uint8* l_NextAddress = PLAT_START_ADDR;

result
Plat_Virtual_Alloc(
    uint64          size_In,
    uint64*         pSizeAct_Out,
    const void**    pAlloc_Out)
{
    unsigned int pgSize = PageSize()
    size_In += pgSize - (size_In % pgSize);
    uint64* pAlloc = (uint64*)SysAlloc(l_NextAddress, size_In);
    if(pAlloc)
    {
        pAlloc[0] = size_In;
        *pSizeAct_Out = size_In - sizeof(uint64);
        *pAlloc_Out = (pAlloc + 1);
        l_NextAddress += PLAT_ALLOC_BUFFER;
        return PLAT_SUCCESS;
    }
    return PLAT_NOALLOC;
}

result
Plat_Virtual_Realloc(
    uint64          size_In,
    uint64*         pSizeActOut,
    const void*     pAlloc_In)
{
    uint64* toExtend = (uint64*)pAlloc_In - 1;
    void* allocBack = (void*)pAlloc_In + toExtend[0];
    if(size_In >= toExtend[0] && size_In <= PLAT_ALLOC_BUFFER)
    {
        uint64 extensionSize = size_In - toExtend[0];
        unsigned int pgSize = PageSize()
        extensionSize += pgSize - (size_In % pgSize);
        SysAlloc(allocBack, extensionSize);
        toExtend[0] += extensionSize;
        *pSizeActOut = toExtend[0] - sizeof(uint64);
        return PLAT_SUCCESS;
    }
    return (size_In <= toExtend[0]) ? PLAT_NEGEXT : PLAT_BUFFLIM;
}

result
Plat_Virtual_Free(
    uint64          size_In,
    const void**    block_InOut)
{
    uint64* toFree = *(uint64**)block_InOut;
    *(void**)block_InOut = null;
    SysFree((void*)(toFree - 1), size_In);
    return PLAT_SUCCESS;
}

#endif // PLATFORM_ALLOC_H