#ifndef PLATFORM_CORE_H
#define PLATFORM_CORE_H
    #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
        #if defined(_WIN64)
            #define PLATFORM_WINDOWS 1
            #include <Windows.h>
        #else
        #error "Error: Only supported on 64 bit Windows."
        #endif // _WIN64
    #elif defined(__linux__)
        #ifdef __gnu_linux_
            #define PLATFORM_LINUX 1
        #else
        #error "Error: Still working on an inclusive Linux build."
        #endif // __gnu_linux__
    #elif defined(__APPLE__) || defined(__MACH__)
        #define VM_MAC_OS 1
    #else
        #error "Error: Incompatible platform."
    #endif // Platform
#endif // PLATFORM_CORE_H