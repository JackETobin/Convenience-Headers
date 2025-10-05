// TODO: Add in errors, we need a way to fail gracefully instead of just faceplanting.

#ifndef PLATFORM_H
#define PLATFORM_H

#if defined(MEM_TYPES_H)
    #include "mem_types.h"
#else
typedef unsigned long long uint64;
#endif // MEM_TYPES_H

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

// TODO: Exception has occurred: W32/0xC0000005 <- This occurs seemingly randomly...
// NOTE: Unhandled exception at 0x00007FF7F9B77379 in header_test.exe: 0xC0000005: Access violation writing location 0x000000BB875C0070.
void
Plat_Virtual_Alloc(
    uint64          size_In,
    void*           pStart_In,
    uint64*         pSizeAct_Out,   // <- These two are sometimes null, take a look at the call...
    const void**    pAlloc_Out)     // <- These two are sometimes null, take a look at the call... 
{
    unsigned int pgSize = PageSize()
    size_In += pgSize - (size_In % pgSize);
    void* pAlloc = SysAlloc(pStart_In, size_In);
    if(pAlloc)
    {
        *pSizeAct_Out = size_In;
        *pAlloc_Out = pAlloc;
    }
    return;
}

void*
Plat_Virtual_Free(
    uint64          size_In,
    void*           pBlock_In)
{
    SysFree(pBlock_In, size_In);
    return (void*)0;
}

#endif // PLATFORM_H