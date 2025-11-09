#ifndef MEM_POOL_H
#define MEM_POOL_H

#if !defined(MEM_HEADERS_H) || !defined(PLATFORM_H)
    #define null                ((void*)0)
    #define true                (uint8)0X01
    #define false               (uint8)0X00

    typedef unsigned char       uint8;
    typedef unsigned short      uint16;
    typedef unsigned int        uint32;
    typedef unsigned long long  uint64;

    typedef uint8               result;
#endif // MEM_TYPES
#if !defined(DEFINES_H)
    #define persist             static
    #define internal            static

    #define unused(x)           (void)(x)
#endif // DEFINES_H

#define POOL_ALIGN_MIN          (3 * sizeof(void*))

#define POOL_REL_BACK            0
#define POOL_REL_PREV           1
#define POOL_REL_NEXT           2

#define POOL_SUCCESS            (result)0X02 // memory pool success.
#define POOL_NOSPACE            (result)0X19 // Not enough space available.
#define POOL_NOPOOL             (result)0X1A // No available pools, call Pool_Build().
#define POOL_NOSIZE             (result)0X1B // No size provided.
#define POOL_NOALLOC            (result)0X1C // Unable to allocate space.
#define POOL_NOFREE             (result)0X1D // No free provided.
#define POOL_INVDATA            (result)0X1E // Invalid pointer to data.
#define POOL_INVRANGE           (result)0X1F // Invalid range.
#define POOL_NORESIZE           (result)0X20 // Pool is not resizeable.
#define POOL_NOOUTPUT           (result)0X21 // No output buffer provided.
#define POOL_INVPLHND           (result)0X22 // Invalid pool handle.
#define POOL_MISALIGNED         (result)0X23 // Misaligned pointer to data.
#define POOL_NOITOA             (result)0X24 // Reservation has been released.
#define POOL_NOCONCAT           (result)0X25 // Release has already been called on this reservation.
// #define POOL_RESNOFIT        (result)0X26 // Reservation isn't large enough, data has been truncated.
#define POOL_INVOFFST           (result)0X27 // Invalid buffer offset on write attempt.
// #define POOL_RESFAULT        (result)0x28 // Unable to obtain the reservation.
#define POOL_RESIZE             (result)0X29 // Pool has been resized and a raw pointer update might be necessary.

#if defined(MODE_DEBUG) && !defined(MEM_HEADERS_H)
    #include "../log.h"
    #define DBG(func) _Debug_Catch(func, #func, line, file)
#elif !defined(MEM_HEADERS_H)
    #define DBG(func) func
#endif // MODE_DEBUG

typedef uint16      pool_hnd;

typedef struct Pool_St
{
    struct Pool_St* pPrev;
    uint64          size;
    void*           pData;
    void**          pRelHead;
    uint32          alignment;
    uint8           dynamic;
    pool_hnd        hnd;
} pool;

persist pool* l_PoolHead = null;

internal uint64
l_pool_align(
    uint64          size_In, 
    uint32          align_In)
{
    uint32 adjust = (size_In % align_In);
    size_In += (adjust) ? align_In - adjust : 0;
    return size_In;
}

#if defined(USE_MEM_ALLOC) || defined (USE_MEM_ALL)
    #include "mem_alloc.h"

    #define POOL_TAG_MAX        ALLOC_TAG_SIZE

    internal result 
    l_pool_alloc(
        uint64*     size_InOut, 
        uint32      align_In, 
        char*       tag_In, 
        void**      mem_Out)
    { return _Mem_Alloc_Allign(size_InOut, align_In, tag_In, (p_mem)mem_Out); }

    internal result
    l_pool_realloc(
        uint64*     size_InOut, 
        uint32      align_In,
        void**      mem_InOut)
    {
        unused(align_In);
        return _Mem_Realloc(size_InOut, (p_mem)mem_InOut); 
    }

    internal result
    l_pool_free(
        void**      mem_In)
    { return _Mem_Free((p_mem)mem_In); }

    internal result
    l_pool_kill()
    { return _Mem_Kill(); }
#else
    #include <stdlib.h>
    
    #define POOL_TAG_MAX        48

    internal result
    l_pool_alloc(
        uint64*     size_InOut, 
        uint32      align_In, 
        char*       tag_In, 
        void**      mem_Out)
    {
        unused(tag_In); 
        if(!size_InOut || !*size_InOut)
            return POOL_NOSIZE;
        if(!mem_Out)
            return POOL_NOOUTPUT;
            
        uint64 size = l_pool_align(*size_InOut, align_In);
        void* new = malloc(size);
        if(new)
        {
            *size_InOut = size;
            *mem_Out = new;
            return POOL_SUCCESS;
        }
        return POOL_NOALLOC;
    }

    internal result
    l_pool_realloc(
        uint64*     size_InOut,
        uint32      align_In,
        void**      mem_InOut)
    {
        if(!size_InOut || !*size_InOut)
            return POOL_NOSIZE;
        if(!mem_InOut)
            return POOL_NOOUTPUT;
        
        uint64 size = l_pool_align(*size_InOut, align_In);
        void* new = realloc(*mem_InOut, size);
        if(new)
        {
            *size_InOut = size;
            *mem_InOut = new;
            return POOL_SUCCESS;
        }
        return POOL_NORESIZE;
    }

    internal result
    l_pool_free(
        void**      mem_In)
    {
        free(mem_In);
        *mem_In = null;
        return POOL_SUCCESS;
    }

    internal result
    l_pool_kill()
    {
        while(l_PoolHead)
        {
            pool* toFree = l_PoolHead;
            l_PoolHead = l_PoolHead->pPrev;
            l_pool_free(&toFree);
        }
        return POOL_SUCCESS;
    }
#endif // USE_MEM_ALLOC || USE_MEM_ALL
#if defined(USE_LOCAL_STRING)
    #include "../string.h"

    internal result
    l_pool_itoa(
        uint64      num_In, 
        uint32      buffSize_In, 
        char*       str_Out)
    { return _String_Itoa(num_In, buffSize_In, str_Out); };

    internal result
    l_pool_concat(
        char**      strArray_In, 
        uint32      count_In, 
        uint32      buffSize_In, 
        char*       str_Out)
    { return _String_Concat(strArray_In, count_In, buffSize_In, str_Out); };
#else
    #include <stdlib.h>
    #include <string.h>

    internal result
    l_pool_itoa(
        uint64      num_In, 
        uint32      buffSize_In, 
        char*       str_Out)
    {
        if(_itoa_s(num_In, str_Out, buffSize_In, 10))
            return POOL_NOITOA;
        return POOL_SUCCESS;
    }

    internal result
    l_pool_concat(
        char**      strArray_In, 
        uint32      count_In, 
        uint32      buffSize_In, 
        char*       str_Out)
    {
        unused(count_In);
        strcpy(str_Out, strArray_In[0]);
        if(strcat_s(str_Out, buffSize_In, strArray_In[1]))
            return POOL_NOCONCAT;
        return POOL_SUCCESS;
    }
#endif // STRING_H

/*
 * FUNCTION: Builds a pool of at least the specified size.
 */
result
_Pool_Build(
    uint64*         size_InOut, 
    uint32          align_In, 
    uint8           dynamicFlag_In, 
    pool_hnd*       poolHnd_Out)
{
    if(!size_InOut || !*size_InOut)
            return POOL_NOSIZE;
    if(!poolHnd_Out)
        return POOL_NOOUTPUT;

    uint32 align = (align_In >= POOL_ALIGN_MIN) ? align_In : POOL_ALIGN_MIN;
    pool_hnd hnd = (l_PoolHead) ? l_PoolHead->hnd + 1 : 1;
    char tag[POOL_TAG_MAX];
    char hndStr[8];
    DBG(l_pool_itoa(hnd, 8, hndStr));
    DBG(l_pool_concat((char* []){"Pool ", hndStr}, 2, POOL_TAG_MAX, tag));

    pool* new = null;
    *size_InOut += align + sizeof(*new);
    DBG(l_pool_alloc(size_InOut, align, tag, (void**)&new));
    if(new)
    {
        void* data = new + sizeof(*new);
        uint32 ptrAdjust = (uint64)data % align;
        data += (ptrAdjust) ? align - ptrAdjust : 0;
        *new = (pool){l_PoolHead, *size_InOut, data, null, align, dynamicFlag_In, hnd};
        l_PoolHead = new;

        *poolHnd_Out = hnd;
        return POOL_SUCCESS;
    }
    return POOL_NOALLOC;
};

/*
 * FUNCTION: Destroys pool corresponding to the specified handle.
 */
result
_Pool_Destroy(
    pool_hnd*       poolHnd_In)
{
    pool* toFree = l_PoolHead;
    pool* next = toFree;
    while(toFree->hnd != *poolHnd_In)
    {
        toFree = toFree->pPrev;
        next = (toFree->hnd != *poolHnd_In) ? toFree : next;
    }
    next->pPrev = toFree->pPrev;
    l_PoolHead = (toFree == l_PoolHead) ? l_PoolHead->pPrev : l_PoolHead;
    l_pool_free((void**)&toFree);

    *poolHnd_In = 0;
    return POOL_SUCCESS;
};

/*
 * FUNCTION: Destroys all active pools.
 */
result
_Pool_Kill()
{ return l_pool_kill(); };

/*
 * FUNCTION: Claims space of at least the specified size if possible.
 */
result
_Pool_Claim(
    pool_hnd        poolHnd_In, 
    uint64*         size_InOut, 
    void**          data_Out)
{
    // NOTE: This should check for void space before returning pNext and moving pNext up.
    pool* target = l_PoolHead;
    while(target->hnd != poolHnd_In)
        target = target->pPrev;
    if(!target)
        return POOL_INVPLHND;

    uint64 size = l_pool_align(*size_InOut, target->alignment);
    void** new = target->pRelHead;
    
    while(new && (uint64)(new[POOL_REL_BACK] - (void*)new) < size)
        new = (void**)new[POOL_REL_NEXT];
    uint64 sizeRel = (new) ? new[POOL_REL_BACK] - (void*)new : 0;
    if(new)
    {
        if(sizeRel - size >= target->alignment)
        {
            new[POOL_REL_BACK] -= size;
            new += (sizeRel - size);
        }
        else
        {
            ((void**)new[POOL_REL_PREV])[POOL_REL_NEXT] = new[POOL_REL_NEXT];
            ((void**)new[POOL_REL_NEXT])[POOL_REL_PREV] = new[POOL_REL_PREV];
        }
    }
    uint8 newClaim = (new == null);
    *data_Out = (newClaim) ? target->pData : new;
    target->pData += newClaim * size;
    
    *size_InOut = size;
    return POOL_SUCCESS;
};

/*
 * FUNCTION: Releases a specified range of data.
 * // NOTE: Range should be aligned/
 */
result
_Pool_Release(
    pool_hnd        poolHnd_In, 
    void**          data_InOut,
    uint64          range_In)
{
    // TODO: We really need to find a better way to coalesce. Reconsider the order of operations here....
    // NOTE: This should coalesce adjacent spaces, and move pNext back if possible.
    // NOTE: How are we going to handle non-aligned data ranges??
    // - we could require that the pointer to the range begining be aligned, then just automatically align the range?
    pool* target = l_PoolHead;
    while(target->hnd != poolHnd_In)
        target = target->pPrev;
    if(!target)
        return POOL_INVPLHND;

    uint64 range = l_pool_align(range_In, target->alignment);
    void** pData = *data_InOut;
    if((void*)pData + range > target->pData)
        return ((void*)pData > target->pData - target->alignment) ? POOL_INVDATA : POOL_INVRANGE;
    if((uint64)pData % target->alignment)
        return POOL_MISALIGNED;
    
    pData[POOL_REL_BACK] = (void*)pData + range;
    pData[POOL_REL_PREV] = pData[POOL_REL_NEXT] = null;
    void** prev = target->pRelHead;
    if(prev) // Check whether we have a starting entry in the released space linked list.
    {
        while(prev[POOL_REL_NEXT] && prev[POOL_REL_NEXT] < (void*)pData)
            prev = prev[POOL_REL_NEXT];
        void** next = prev[POOL_REL_NEXT];
        // Coalesce previous entry and newly released space.
        if(prev[POOL_REL_BACK] == pData)
        {
            prev[POOL_REL_BACK] = pData[POOL_REL_BACK];
            pData = prev;
        }
        // TODO: Bruh, this is an absolute mess...there has to be a better way...
        // coalesce next entry and newly released space.
        if(pData[POOL_REL_BACK] == next)
        {
            pData[POOL_REL_BACK] = next[POOL_REL_BACK]; // New space back is set to next space back.
            pData[POOL_REL_NEXT] = next[POOL_REL_NEXT]; // New space next is set to next space next.
            if(pData != prev) // Check whether new space was coalesced into previous space.
                prev[POOL_REL_NEXT] = pData; // If not coalesced, link new space to previous space's next entry.
            if(pData[POOL_REL_NEXT]) // Check whether next entry is linked.
                ((void**)pData[POOL_REL_NEXT])[POOL_REL_PREV] = pData; // If next entry is linked, set next entry's previous to new entry. 
            next = pData; // Set next equal to new to denote coalescence.
        }
        if(pData != prev && pData != next) // Check whether new space was coalesced into previous or next spaces.
        { // If the new space wasnt coalesced, link it into the released space linked list.
            pData[POOL_REL_PREV] = prev;
            pData[POOL_REL_NEXT] = next;
            prev[POOL_REL_NEXT] = pData;
            if(next)
                next[POOL_REL_PREV] = pData;
        }
    }
    else
        prev = target->pRelHead = pData;

    *data_InOut = null;
    return POOL_SUCCESS;
};

#endif // MEM_POOL_H