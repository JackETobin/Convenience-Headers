// TODO: Add in errors, we need a way to fail gracefully instead of just faceplanting.

#ifndef PLATFORM_H
#define PLATFORM_H

typedef unsigned char       uint8;
typedef unsigned long long  uint64;
typedef uint8               result;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
    #if defined(_WIN64)
        #if defined(STD_EXTERN)
            #undef STD_EXTERN
            #define STD_EXTERN __stdcall
        #endif // MEM_DEFINES
        #include "Windows.h"
        typedef DWORD l_page;

        l_page
        QueryPageSize(void)
        {
            SYSTEM_INFO sysInfo = { 0 };
            GetSystemInfo(&sysInfo);
            return sysInfo.dwPageSize;
        }
        #define PageSize() QueryPageSize();
        #define SysAlloc(pStart, size) VirtualAlloc(pStart, size,             \
                                                    MEM_COMMIT | MEM_RESERVE, \
                                                    PAGE_READWRITE)

        #define SysFree(pBlock, size) VirtualFree(pBlock, size, MEM_RELEASE)
    #else
    #error "Error: Only supported on 64 bit Windows."
    #endif // _WIN64
#elif defined(__linux__)
    #ifdef __gnu_linux_
    
        #include <sys/mman.h>
        #include <unistd.h>
        typedef int l_page;

        #define PageSize() getpagesize()
        #define SysAlloc(pStart, size) mmap(pStart, size,                   \
                                            PROT_READ | PROT_WRITE,         \
                                            MAP_PRIVATE | MAP_ANONYMOUS,    \
                                            0, 0)

        #define SysFree(pBlock, size) munmap(pBlock, size)
    #else
    #error "Error: Still working on a Linux build."
    #endif // __gnu_linux__
#elif defined(__APPLE__) || defined(__MACH__)
    #define VM_MAC_OS 1
#else
    #error "Error: Incompatible platform."
#endif // Platform

#define PLAT_SUCCESS (result)0X1C // Platform success.
#define PLAT_NOALLOC (result)0X1D // Platform unable to allocate memory.

result
Plat_Virtual_Alloc(
    uint64          size_In,
    void*           pStart_In,
    uint64*         pSizeAct_Out,
    const void**    pAlloc_Out)
{
    unsigned int pgSize = PageSize()
    size_In += pgSize - (size_In % pgSize);
    void* pAlloc = SysAlloc(pStart_In, size_In);
    if(pAlloc)
    {
        *pSizeAct_Out = size_In;
        *pAlloc_Out = pAlloc;
        return PLAT_SUCCESS;
    }
    return PLAT_NOALLOC;
}

result
Plat_Virtual_Free(
    uint64          size_In,
    const void**    block_InOut)
{
    SysFree(*(void**)block_InOut, size_In);
    *(void**)block_InOut = (void*)0;
    return PLAT_SUCCESS;
}

#endif // PLATFORM_H