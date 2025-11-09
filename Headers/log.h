#ifndef LOG_H
#define LOG_H

#include "defines.h"
#include "Platform_Headers/platform_types.h"

// #define PLAT_SUCCESS        (result)0X00 // Platform success.
// #define ALLOC_SUCCESS       (result)0X01 // Operation succesful.
// #define POOL_SUCCESS        (result)0X02 // memory pool success.
// #define ARRAY_SUCCESS       (result)0X03 // array success.

// MEM_ALLOC_H
// #define ALLOC_NOISSUE       (result)0X10 // No overwrite issues detected.
// #define ALLOC_OUTOFBOUNDS   (result)0X11 // Memory overwrite detected.
// #define ALLOC_NOALLOC       (result)0X12 // Unable to allocate memory.
// #define ALLOC_NOFREE        (result)0X12 // Unable to free memory.
// #define ALLOC_NOOUTPUT      (result)0X14 // No output buffer provided.
// #define ALLOC_NOINPUT       (result)0X15 // Input field missing.
// #define ALLOC_UNUSED        (result)0X16 // No allocations made as of yet.
// #define ALLOC_NOPREV        (result)0X17 // No previous allocation to iterate to.
// #define ALLOC_NONEXT        (result)0X18 // No next allocation to iterate to.

// MEM_POOL_H
// #define POOL_NOSPACE        (result)0X19 // Not enough space available.
// #define POOL_NOPOOL         (result)0X1A // No avaolable pools, call Pool_Build().
// #define POOL_NOSIZE         (result)0X1B // No size provided.
// #define POOL_NOALLOC        (result)0X1C // No allocator provided.
// #define POOL_NOFREE         (result)0X1D // No free provided.
// #define POOL_NOBUILD        (result)0X1E // Unable to allocate space.
// #define POOL_NOACTIVE       (result)0X1F // Pool has been freed already.
// #define POOL_NORESIZE       (result)0X20 // Pool is not resizeable.
// #define POOL_NOOUTPUT       (result)0X21 // No output buffer provided.
// #define POOL_INVPLHND       (result)0X22 // Invalid pool handle.
// #define POOL_INVREHND       (result)0X23 // Invalid reservation handle.
// #define POOL_RESFREE        (result)0X24 // Reservation has been released.
// #define POOL_DBLFREE        (result)0X25 // Release has already been called on this reservation.
// #define POOL_RESNOFIT       (result)0X26 // Reservatioin isn't large enough, data has been truncated.
// #define POOL_INVOFFST       (result)0X27 // Invalid buffer offset on write attempt.
// #define POOL_RESFAULT       (result)0x28 // Unable to obtain the reservation.
// #define POOL_RESIZE         (result)0X29 // Pool has been resized and a raw pointer update might be necessary.

// PLATFORM_H
// #define PLAT_NOALLOC        (result)0X2A // Platform unable to allocate memory.
// #define PLAT_NEGEXT         (result)0X2B // Platform realloc cannot shrink an allocation.
// #define PLAT_BUFFLIM        (result)0X2C // Platform virtual alloc extension cap reached.

// MEM_ARRAY_H
// #define ARRAY_NOALLOC       (result)0X2D // Unable to allocate space for the array.
// #define ARRAY_NOFREE        (result)0X2E // An error ocurred while attempting to free the array.
// #define ARRAY_NORESIZE      (result)0X2F // Array was unable to dynamically resize.
// #define ARRAY_EMPTY         (result)0X30 // Array is empty.

#endif // LOG_H

char* l_DebugMsg[] = {
    "Platform success.",
    "Operation succesful.",
    "memory pool success.",
    "array success.",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "No overwrite issues detected.",
    "Memory overwrite detected.",
    "Unable to allocate memory.",
    "Unable to free memory.",
    "No output buffer provided.",
    "Input field missing.",
    "No allocations made as of yet.",
    "No previous allocation to iterate to.",
    "No next allocation to iterate to.",
    "Not enough space available.",
    "No avaolable pools, call Pool_Build().",
    "No size provided.",
    "No allocator provided.",
    "No free provided.",
    "Unable to allocate space.",
    "Pool has been freed already.",
    "Pool is not resizeable.",
    "No output buffer provided.",
    "Invalid pool handle.",
    "Invalid reservation handle.",
    "Reservation has been released.",
    "Release has already been called on this reservation.",
    "Reservatioin isn't large enough, data has been truncated.",
    "Invalid buffer offset on write attempt.",
    "Unable to obtain the reservation.",
    "Pool has been resized and a raw pointer update might be necessary.",
    "Platform unable to allocate memory.",
    "Platform realloc cannot shrink an allocation.",
    "Platform virtual alloc extension cap reached.",
    "Unable to allocate space for the array.",
    "An error ocurred while attempting to free the array.",
    "Array was unable to dynamically resize.",
    "Array is empty."
};

#if defined(PLATFORM_H) || defined(MEM_USE_ALL)
    #include <stdio.h>
    #define Print(s, ...) printf(s, __VA_ARGS__);
#else
    #include <stdio.h>
    #define Print(s, ...) printf(s, __VA_ARGS__);
#endif // PLATFORM_H

result
_Debug_Catch(
    result          res_In, 
    char*           call_In, 
    int32           line_In, 
    char*           file_In)
{
#if defined(DEBUG_LOG_RESULT)
    #if !defined(DEBUG_LOG_VERBOSE)
        if(res_In < 0X10)
            return res_In;
    #endif // DEBUG_LOG_VERBOSE

        Print("File: %s\nLine: %d\nCall: %s\nResult: %s\n\n", 
            file_In, line_In, call_In, l_DebugMsg[res_In]);
#else
    unused(call_In);
    unused(line_In);
    unused(file_In);
#endif // DEBUG_LOG_RESULT
    return res_In;
};