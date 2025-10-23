/*
 * Mem headers wraps function calls from the headers present in the Mem_Headers folder that should
 * be present along with this header. This header is configurable with the following pre-processor
 * definitions:
 *  -   USE_MEM_HEADERS - Includes all mem headers as well as platform header located externally.
 *  -   USE_PLATFORM    - Includes "platform_h" located externally.
 *  -   USE_MEM_DEBUG   - Includes "mem_debug.h"
 *  -   USE_MEM_ALLOC   - Includes "mem_alloc.h"
 *  -   USE_MEM_POOL    - Includes "mem_pool.h"
 *  -   USE_MEM_STRING  - Includes "mem_string.h"
 *  -   USE_MEM_ARRAY   - Includes "mem_array.h"
 * 
 * All of the header files included here can be used independantly of one another and of 
 * mem_headers.h. Each header file can be used piecewise, but the debugging functionality will be
 * limited should you request a debug report. As of right now the debug report prints to terminal.
 */
#ifndef MEM_HEADERS_H
#define MEM_HEADERS_H

#include "Mem_Headers/mem_defines.h"
#include "Mem_Headers/mem_types.h"

#if defined(USE_ALL) || defined(USE_MEM_HEADERS)
    #define USE_MEM_ALL
#endif // USE_ALL

#if defined(USE_PLATFORM) || defined(USE_MEM_ALL)
    #include "platform.h"
#endif // USE_PLATFORM

#if defined(USE_MEM_DEBUG) || defined(USE_MEM_ALL)
    #include "Mem_Headers/mem_debug.h"

    #define Mem_DBG(func) \
        _Debug_Catch(func, #func, line, file)

    #define Mem_Debug_Report() \
        _Debug_Report()
#else
    #define Mem_DBG(func) func

    #define Mem_Debug_Report()

#endif // USE_MEM_DEBUG
#if defined(USE_MEM_ALLOC) || defined(USE_MEM_ALL)
    #include "Mem_Headers/mem_alloc.h"
    
    #define Mem_Alloc(size_In, tag_In, mem_Out) \
        Mem_DBG(_Mem_Alloc(size_In, tag_In, mem_Out))
    
    #define Mem_Alloc_Allign(size_In, align_In, tag_In, mem_Out) \
        Mem_DBG(_Mem_Alloc_Allign(size_In, align_In, tag_In, mem_Out))
    
    #define Mem_Realloc(size_In, mem_InOut) \
        Mem_DBG(_Mem_Realloc(size_In, mem_InOut))
    
    #define Mem_Free(mem_InOut) \
        Mem_DBG(_Mem_Free(mem_InOut))
    
    #define Mem_BlockTag(mem_In, tag_Out) \
        Mem_DBG(_Mem_BlockTag(mem_In, tag_Out))
    
    #define Mem_Zero(mem_In, range_In) \
        Mem_DBG(_Mem_Zero(mem_In, range_In))
    
    #define Mem_CheckBounds(mem_In) \
        Mem_DBG(_Mem_CheckBounds(mem_In))
    
    #define Mem_Next(mem_InOut) \
        Mem_DBG(_Mem_Next(mem_InOut))
    
    #define Mem_Prev(mem_InOut) \
        Mem_DBG(_Mem_Prev(mem_InOut))
    
    #define Mem_ClearFree() \
        Mem_DBG(_Mem_ClearFree())
    
    #define Mem_Kill() \
        Mem_DBG(_Mem_Kill())
    
    #define Mem_BlockSize(mem_In) \
        _Mem_BlockSize(mem_In)

#endif // USE_MEM_ALLOG
#if defined(USE_MEM_POOL) || defined(USE_MEM_ALL)
    #include "Mem_Headers/mem_pool.h"

    /* 
    * FUNCTION: Builds a memory pool.
    *  - input  - size_In:      byte size of the pool to be built, will be aligned to the 
    *                           POOL_SECTION_SIZE.
    *  - input  - tag_In:       memory tag for the pool, requires MEM_ALLOC_H.
    *  - input  - resize_In:    flag that indicates whether or not the pool can be resized.
    *  - output - poolHnd_Out:  output buffer to be filled with the handle of the newly built pool.
    * 
    * Each pool consists of two allocations: one that is the requested memory space and the space 
    * metadata, and second allocation for the free-list that can grow dynamically.
    */
    #define Pool_Build(size_In, tag_In, resize_In, poolHnd_Out) \
        Mem_DBG(_Pool_Build(size_In, tag_In, resize_In, poolHnd_Out))
    
    /* 
    * FUNCTION: Destroys a designated memory pool.
    *  - input  - poolHnd_In:   a handle to a pool designated for destruction.
    *
    * Pools are not completely disgarded; they are resized to just the meta-data, and the free list 
    * is actually freed. This is to preserve the pool index so that the indices for active pools 
    * remain unchanged.
    */
    #define Pool_Destroy(poolHnd_In) \
        Mem_DBG(_Pool_Destroy(poolHnd_In))
    
    /* 
    * FUNCTION: Frees all allocations associated with mem_pool.h
    *  - no input -
    * 
    * Nullifies the linked list used internally to access allocations.
    */
    #define Pool_Kill() \
        Mem_DBG(_Pool_Kill())
    
    /* 
    * FUNCTION: Resizes a designated pool.
    *  - input  - poolHnd_In:   a handle of a pool designated for resizing.
    *  - input  - size_In:      byte size of the space to be reserved.
    * 
    * Realloc is called, and the contents of the pool are preserved in the new pool. No changes 
    * occur in the pool handle, as the pool's index is preserved.
    */
    #define Pool_Resize(poolHnd_In, size_In) \
        Mem_DBG(_Pool_Resize(poolHnd_In, size_In))
    
    /* 
    * FUNCTION: Reserves space in a memory pool.
    *  - input  - size_In:      byte size of the space to be reserved.
    *  - output - resHnd_Out:   buffer to be filled with a handle corresponding the the newly 
    *                           reserved memory.
    * 
    * Reservations are made in the first available space in any pool; does not provide control over 
    * where reservations are made.
    */
    #define Pool_Reserve(size_In, resHnd_Out) \
        Mem_DBG(_Pool_Reserve(size_In, resHnd_Out))
    
    /*
    * FUNCTION: Reserves space in a memory pool.
    *  - input  - size_In:      bytesize of the space to be reserved.
    *  - input  - poolHnd_In:   handle of the pool in which a reservation is to be made.
    *  - output - resHnd_Out:   buffer to be filled with a handle corresponding the the newly 
    *                           reserved memory.
    * 
    * Reservations are made in a designated pool. This function will not attempt to find space in 
    * another location should the designated pool be full.
    */
    #define Pool_Reserve_At(size_In, poolHnd_In, resHnd_Out) \
        Mem_DBG(_Pool_Reserve_At(size_In, poolHnd_In, resHnd_Out))
    
    /* 
    * FUNCTION: Releases a reservation, allowing for reallocation and preventing further writing.
    *  - input  - resHnd_In:    a handle to the designated reservation intended to be released.
    *
    * Data within the reservation is not destroyed, the reservation is simply flagged as released 
    * and if possible, coalesced into adjacent released reservations.
    */
    #define Pool_Release(resHnd_In) \
        Mem_DBG(_Pool_Release(resHnd_In))
    
    /* 
    * FUNCTION: Retrieves a raw pointer to a reservation's data.
    *  - input  - resHnd_In:    a handle to the designated reservation intended to be retrieved.
    *  - output - rawPtr_Out:   a buffer to be filled with a pointer to reservation data.
    * 
    * Since this returns pointers to data, writing into the reservation via the raw pointer is 
    * possible, however there is not way to guarantee that the reservatioin wont be overwritten 
    * should you use the pointer to fill the reservation. Request the size beforehand to check how 
    * much space is actually present.
    */
    #define Pool_Retrieve(resHnd_In, rawPtr_Out) \
        Mem_DBG(_Pool_Retrieve(resHnd_In, rawPtr_Out))
    
    /* 
    * FUNCTION: Copies data into a pool reservation.
    *  - input  - resHnd_In:    a handle to the reservation designated to be written to.
    *  - input  - data_In:      a pointer to the source data to be written.
    *  - input  - dataSize_In:  the size of the source data to be written.
    *  - input  - offest_In:    the offset into the destination reservation where data is to be 
    *                           written.
    * 
    * This function prevents overwriting the reservation. If dataSize_In is zero, the function will 
    * write to the end of the reservation. It will read past the source data buffer should the 
    * buffer be smaller than the reservation.
    */
    #define Pool_Write(resHnd_In, data_In, dataSize_In, offset_In) \
        Mem_DBG(_Pool_Write(resHnd_In, data_In, dataSize_In, offset_In))
    
    /* 
    * FUNCTION: Modifies the size of a reservation.
    *  - InOut  - resHnd_InOut: a buffer containing a handle to the reservation designated for 
    *                           resizing.
    *  - input  - size_In:      the new size of the reservation.
    * 
    * InOut reservation handle is modified and the new handle is written into the InOut buffer 
    * provided.
    */
    #define Pool_Modify(resHnd_InOut, size_In) \
        Mem_DBG(_Pool_Modify(resHnd_In, size_In))

#endif // USE_MEM_POOL
 // TODO: Setup wrappers for string functions.
#if defined(USE_MEM_STRING) || defined(USE_MEM_ALL)
    #include "Mem_Headers/mem_string.h"

    result
    _String_Itoa(
        int64           int64_In, 
        uint32          buffSize_In, 
        char*           str_Out);

    result
    _String_TrimLeading(
        char*           str_In,
        uint32          buffSize_In,
        char*           buff_Out);

    result
    _String_RemoveSpaces(
        char*           str_In,
        uint32          buffSize_In,
        char*           buff_Out);

    result
    _String_Concat(
        char*           strArray_In[], 
        uint32          count_In, 
        uint32          buffSize_In, 
        char*           str_Out);
#endif
// TODO: Write documentation for the array functions, and get them into the array header.
#if defined(USE_MEM_ARRAY) || defined(USE_MEM_ALL)
    #include "Mem_Headers/mem_array.h"

    #if defined(USE_MEM_POOL) || defined(USE_MEM_ALL)

        #define Array_DesignatePool(poolHnd_In) \
            MemDBG(_Array_DesignatePool(poolHnd_In))
            
    #endif

    #define Array_Build(stride_In, count_In, hnd_Out) \
        Mem_DBG(_Array_Build(stride_In, count_In, &hnd_Out))

    #define Array_Destroy(hnd_In) \
        Mem_DBG(_Array_Destroy(&hnd_In))
        
    #define Array_Push(hnd_InOut, data_In) \
        Mem_DBG(_Array_Push(&hnd_InOut, (void*)&data_In))
        
    #define Array_PushBack(hnd_InOut, data_In) \
        Mem_DBG(_Array_PushBack(&hnd_InOut, (void*)&data_In))
        
    #define Array_Pop(hnd_In, data_Out) \
        Mem_DBG(_Array_Pop(hnd_In, (void*)&data_Out))
        
    #define Array_PopBack(hnd_In, data_Out) \
        Mem_DBG(_Array_PopBack(hnd_In, (void*)&data_Out))

    #define Array_Insert(hnd_In, element_In, data_In) \
        Mem_DBG(_Array_Insert(&hnd_In, element_In, (void*)&data_In))
        
    #define Array_Remove(hnd_In, element_In) \
        Mem_DBG(_Array_Remove(hnd_In, element_In))
        
    #define Array_Length(hnd_In, len_Out) \
        Mem_DBG(_Array_Length(hnd_In, &len_Out))
        
    #define Array_Size(hnd_In, size_Out) \
        Mem_DBG(_Array_Size(hnd_In, &size_Out))
        
    #define Array_Stride(hnd_In, stride_Out) \
        Mem_DBG(_Array_Stride(hnd_In, &stride_Out))

#endif // USE_MEM_ARRAY

#endif // MEM_HEADERS_H