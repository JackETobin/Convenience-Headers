#ifndef MEM_POOL_H
#define MEM_POOL_H

#if !defined(MEM_TYPES_H)
    #define null                ((void*)0)
    #define true                (uint8)0X01
    #define false               (uint8)0X00

    typedef unsigned char       uint8;
    typedef unsigned short      uint16;
    typedef unsigned int        uint32;
    typedef unsigned long long  uint64;

    typedef double              float64;
    typedef uint8               result;
#endif // USE_MEM_TYPES

#if !defined(MEM_DEFINES_H)
    #define persist         static
    #define internal        static

    #define packed          __attribute__((__packed__))

    #define unused(x)       (void)(x)

    #define POOL_SUCCESS    (result)0X0A // memory manager success.
    #define POOL_NOSPACE    (result)0X0B // No space available.
    #define POOL_NOPOOL     (result)0X0C // No avaolable pools, call Pool_Build().
    #define POOL_NOSIZE     (result)0X0D // No size provided.
    #define POOL_NOALLOC    (result)0X0E // No allocator provided.
    #define POOL_NOFREE     (result)0X0F // No free provided.
    #define POOL_NOBUILD    (result)0X10 // Unable to allocate space.
    #define POOL_NOACTIVE   (result)0X11 // Pool has been freed already.
    #define POOL_NORESIZE   (result)0X12 // Pool is not resizeable.
    #define POOL_NOOUTPUT   (result)0X13 // No output buffer provided.
    #define POOL_INVPLHND   (result)0X14 // Invalid pool handle.
    #define POOL_INVREHND   (result)0X15 // Invalid reservation handle.
    #define POOL_RESFREE    (result)0X16 // Reservation has been released.
    #define POOL_DBLFREE    (result)0X17 // Release has already been called on this reservation.
    #define POOL_RESNOFIT   (result)0X18 // Reservatioin isn't large enough, data has been truncated.
    #define POOL_INVOFFST   (result)0X19 // Invalid buffer offset on write attempt.
    #define POOL_RESFAULT   (result)0x1A // Unable to obtain the reservation.
    #define POOL_FAILURE    (result)0X1B // Memory manager critical failure.
#endif // USE_MEM_DEFINES

// typedef             int   (*pfn_print)      (const char *const, ...);

#define POOL_ALIGN 8

// #if defined(USE_MEM_DEBUG) || defined(USE_MEM_ALL)
//     #include "mem_debug.h"
// #else
//     #define DBG(func) func
// #endif

#if defined(USE_MEM_ALLOC) || defined (USE_MEM_ALL)
    #include "mem_alloc.h"
    #define Pool_Alloc(size_In, tag_In, mem_Out) _Mem_Alloc_Allign(size_In, POOL_ALIGN, tag_In, (p_mem)&mem_Out)
    #define Pool_Realloc(size_In, mem_InOut)     _Mem_Realloc(size_In, (p_mem)&mem_InOut)
    #define Pool_Free(mem_In)                    _Mem_Free((p_mem)&mem_In)
    #define Pool_Free_All()                      _Mem_Kill();
#elif defined(USE_CUSTOM_ALLOCATOR)
#else
    #include <stdlib.h>
    #define Pool_Alloc(size_In, tag_In, mem_Out) mem_Out = malloc(size_In)
    #define Pool_Realloc(size_In, mem_InOut)     mem_InOut = realloc(mem_InOut, size_In)
    #define Pool_Free(mem_In)                    free(mem_In)
#endif // MEM_ALLOC_H || CUSTOM_ALLOCATOR

#define MEM_ASSERT(x)       x
#define POOL_SECTION_SIZE   32

#define POOL_TOP(t) (void*)((void*)t + sizeof(*t))

#define RES_MIN_SIZE                    3
#define RES_META_SIZE                   (4 * sizeof(uint32))

#define RES_HND_ASSEMBLE(r, p, hnd)     (hnd |= (r << 16) | p)
#define RES_HND_OFFSET(r, hnd)          (r = (hnd >> 16))
#define RES_HND_POOL(p, hnd)            (p = (uint16)hnd)

#define RES_SIZE_ALIGNTOSEC(align) (align * sizeof(uint32)) / POOL_SECTION_SIZE
#define RES_SIZE_SECTOALIGN(sec) (sec * POOL_SECTION_SIZE) / sizeof(uint32)

#define POOL_VOID_DEFAULT 12

#define POOL_STATE_READY    (uint16)0X01
#define POOL_STATE_FREED    (uint16)0X02
#define POOL_STATE_FULL     (uint16)0X04
#define POOL_STATE_STATIC   (uint16)0X08

#define RES_STATUS_FAULT    (uint32)0X00
#define RES_STATUS_ACTIVE   (uint32)0X01
#define RES_STATUS_FREE     (uint32)0X02

typedef uint16      pool_hnd;
typedef uint64      res_hnd;

typedef struct packed Pool_St
{
    struct Pool_St* pPrev;
    uint32*         voids;
    uint32          resCount;
    uint32          oEnd;
    uint32          oData;
    uint16          state;
    pool_hnd        hnd;
} pool;
_Static_assert(sizeof(pool) <= POOL_SECTION_SIZE, "Error: mem_pool.h is not supported on this architecture.");

persist pool* l_PoolHead = null;

/* 
 * FUNCTION: Builds a memory pool.
 *  - input  - size_In: byte size of the pool to be built, will be aligned to the POOL_SECTION_SIZE.
 *  - input  - tag_In: memory tag for the pool, requires MEM_ALLOC_H.
 *  - input  - resize_In: flag that indicates whether or not the pool can be resized.
 *  - output - poolHnd_Out: output buffer to be filled with the handle of the newly built pool.
 * 
 * Each pool consists of two allocations: one that is the requested memory space and the space metadata,
 * and second allocation for the free-list that can grow dynamically.
 */
result
_Pool_Build(
    uint64          size_In,
    const char*     tag_In,
    uint8           resize_In,
    pool_hnd*       poolHnd_Out);

/* 
 * FUNCTION: Destroys a designated memory pool.
 *  - input  - poolHnd_In: a handle to a pool designated for destruction.
 *
 * Pools are not completely disgarded; they are resized to just the meta-data, and the free list is
 * actually freed. This is to preserve the pool index so that the indices for active pools remain
 * unchanged.
 */
result
_Pool_Destroy(
    pool_hnd        poolHnd_In);

/* 
 * FUNCTION: Frees all allocations associated with mem_pool.h
 *  - no input -
 * 
 * Nullifies the linked list used internally to access allocations.
 */
result
_Pool_Kill(void);

/* 
 * FUNCTION: Resizes a designated pool.
 *  - input  - poolHnd_In: a handle of a pool designated for resizing.
 *  - input  - size_In: byte size of the space to be reserved.
 * 
 * Realloc is called, and the contents of the pool are preserved in the new pool. No changes occur in
 * the pool handle, as the pool's index is preserved.
 */
result 
_Pool_Resize(
    pool_hnd        poolHnd_In,
    uint64          size_In);

/* 
 * FUNCTION: Reserves space in a memory pool.
 *  - input  - size_In: byte size of the space to be reserved.
 *  - output - resHnd_Out: buffer to be filled with a handle corresponding the the newly reserved memory.
 * 
 * Reservations are made in the first available space in any pool; does not provide control over where
 * reservations are made.
 */
result
_Pool_Reserve(
    uint64          size_In,
    res_hnd*        resHnd_Out);

/*
 * FUNCTION: Reserves space in a memory pool.
 *  - input  - size_In: bytesize of the space to be reserved.
 *  - input  - poolHnd_In: handle of the pool in which a reservation is to be made.
 *  - output - resHnd_Out: buffer to be filled with a handle corresponding the the newly reserved memory.
 * 
 * Reservations are made in a designated pool. This function will not attempt to find space in another location
 * should the designated pool be full.
 */
result
_Pool_Reserve_At(
    uint64          size_In, 
    pool_hnd        poolHnd_In, 
    res_hnd*        resHnd_Out);

/* 
 * FUNCTION: Releases a reservation, allowing for reallocation and preventing further writing.
 *  - input  - resHnd_In: a handle to the designated reservation intended to be released.
 *
 * Data within the reservation is not destroyed, the reservation is simply flagged as released and
 * if possible, coalesced into adjacent released reservations.
 */
result
_Pool_Release(
    res_hnd         resHnd_In);

/* 
 * FUNCTION: Retrieves a raw pointer to a reservation's data.
 *  - input  - resHnd_In: a handle to the designated reservation intended to be retrieved.
 *  - output - rawPtr_Out: a buffer to be filled with a pointer to reservation data.
 * 
 * Since this returns pointers to data, writing into the reservation via the raw pointer is possible,
 * however there is not way to guarantee that the reservatioin wont be overwritten should you use the
 * pointer to fill the reservation. Request the size beforehand to check how much space is actually 
 * present.
 */
result
_Pool_Retrieve(
    res_hnd         resHnd_In, 
    void**          rawPtr_Out);

/* 
 * FUNCTION: Copies data into a pool reservation.
 *  - input  - resHnd_In: a handle to the reservation designated to be written to.
 *  - input  - data_In: a pointer to the source data to be written.
 *  - input  - dataSize_In: the size of the source data to be written.
 *  - input  - offest_In: the offset into the destination reservation where data is to be written.
 * 
 * This function prevents overwriting the reservation. If dataSize_In is zero, the function will write 
 * to the end of the reservation. It will read past the source data buffer should the buffer be smaller 
 * than the reservation.
 */
result
_Pool_Write(
    res_hnd         resHnd_In, 
    void*           data_In, 
    uint64          dataSize_In, 
    uint64          offset_In);

/* 
 * FUNCTION: Modifies the size of a reservation.
 *  - InOut  - resHnd_InOut: a buffer containing a handle to the reservation designated for resizing.
 *  - input  - size_In: the new size of the reservation.
 * 
 * InOut reservation handle is modified and the new handle is written into the InOut buffer provided.
 */
result
_Pool_Modify(
    res_hnd*        resHnd_InOut, 
    uint64          size_In);

#ifndef MEM_ALLOC_H

internal result
l_pool_freeAll()
{
    pool* target = l_Latest;
    while(target)
    {
        pool* toFree = target;
        target = target->pPrev;

        Pool_Free(toFree->voids);
        Pool_Free(toFree);
    }
    l_Latest = null;
    return POOL_SUCCESS;
}
#define Pool_Free_All() l_pool_freeAll()

#endif // MEM_ALLOC_H

internal result
l_pool_obtainRes(
    res_hnd         resHnd_In, 
    uint32**        res_Out,
    uint32*         offset_Out,
    pool**          target_Out)
{
    uint32 offset = 0;
    RES_HND_OFFSET(offset, resHnd_In);
    pool_hnd poolHnd = 0;
    RES_HND_POOL(poolHnd, resHnd_In);

    if(!l_PoolHead)
        return POOL_NOPOOL;

    pool* target = l_PoolHead;
    while(target->hnd != poolHnd && target)
        target = target->pPrev;

    if(!target)
        return POOL_INVREHND;

    uint32* res = (uint32*)((void*)target + (offset * POOL_SECTION_SIZE));
    if(res[0] == RES_STATUS_FAULT)
        return POOL_RESFAULT;
    
    *res_Out = res;
    if(offset_Out)
        *offset_Out = offset;
    if(target_Out)
        *target_Out = target;
    return POOL_SUCCESS;
}

internal uint64
l_pool_setResMeta(uint32* res_In, uint32 status_In, uint32 size_In)
{
    uint64 sizeAlign = RES_SIZE_SECTOALIGN(size_In);
    if(!sizeAlign)
        return sizeAlign;
    res_In[0] = res_In[sizeAlign - 1] = status_In;
    res_In[1] = res_In[sizeAlign - 2] = size_In;
    return sizeAlign;
}

internal uint32
l_pool_reclaim(
    pool*           target_In, 
    uint32*         voidRes_In,
    uint32          voidIndex_In,
    uint32          adjustedSize_In)
{
    uint32 offset = 0;
    uint32* newRes = null;
    uint64 newSizeAlign = 0;

    if(voidRes_In[1] - adjustedSize_In < RES_MIN_SIZE)
    {
        uint32* voidList = target_In->voids + 2;
        offset = voidList[voidIndex_In];
        newRes = voidRes_In;
        newSizeAlign = RES_SIZE_SECTOALIGN(voidRes_In[1]);

        uint32 lim = target_In->voids[1];
        for(uint32 i = voidIndex_In; i < lim; i++)
            voidList[i] = voidList[i + 1];
    }
    else
    {
        voidRes_In[1] -= adjustedSize_In;
        uint64 oldSizeAlign = l_pool_setResMeta(voidRes_In, voidRes_In[0], voidRes_In[1]);
    
        newRes = voidRes_In + oldSizeAlign;
        newSizeAlign = RES_SIZE_SECTOALIGN(adjustedSize_In);
        newRes[1] = newRes[newSizeAlign - 2] = adjustedSize_In;
        offset = ((void*)newRes - (void*)target_In) / POOL_SECTION_SIZE;
    }
    newRes[0] = newRes[newSizeAlign - 1] = RES_STATUS_ACTIVE;
    target_In->resCount++;
    return offset;
}

internal inline void
l_pool_removeVoidEntry(pool* target_In, uint32 offset_In)
{
    uint32 index = 2;
    while(target_In->voids[index] != offset_In && index <= target_In->voids[1])
        index++;
    for(; index < target_In->voids[1] + 2; index++)
        target_In->voids[index] = target_In->voids[index + 1];
    target_In->voids[1]--;
    return;
}

internal uint32
l_pool_reserve(
    pool*           target_In, 
    uint32          adjustedSize_In)
{
    if(adjustedSize_In > (target_In->oEnd - target_In->oData))
        return (uint32)0;
    uint32* res = (void*)target_In + (target_In->oData * POOL_SECTION_SIZE);
    l_pool_setResMeta(res, RES_STATUS_ACTIVE, adjustedSize_In);

    uint32 offset = target_In->oData;
    target_In->oData += adjustedSize_In;
    target_In->resCount++;
    return offset;
}

result
_Pool_Build(
    uint64          size_In,
    const char*     tag_In,
    uint8           resize_In,
    pool_hnd*       poolHnd_Out)
{
#ifndef MEM_ALLOC_H
    unused(tag_In);
#endif // MEM_ALLOC_H
    size_In += (2 * POOL_SECTION_SIZE) - (size_In % POOL_SECTION_SIZE);
    pool* new = null;
    uint32* bucket = null;
    MEM_ASSERT(Pool_Alloc(size_In, tag_In, new));
    MEM_ASSERT(Pool_Alloc((sizeof(*bucket) * (POOL_VOID_DEFAULT + 2)), tag_In, bucket));
    if(!new || !bucket) 
    {
        if(new) MEM_ASSERT(Pool_Free(new));
        if(bucket) MEM_ASSERT(Pool_Free(bucket));
        return POOL_NOBUILD;
    }
    uint32 end = size_In / POOL_SECTION_SIZE;
    uint8 hnd = (l_PoolHead) ? l_PoolHead->hnd + 1 : 0;
    uint8 state = (resize_In) ? POOL_STATE_READY : POOL_STATE_READY | POOL_STATE_STATIC;
    bucket[0] = POOL_VOID_DEFAULT;
    bucket[1] = 0;
    *new = (pool){l_PoolHead, bucket, 0, end, 1, state, hnd};

    l_PoolHead = new;
    *poolHnd_Out = new->hnd;
    return POOL_SUCCESS;
};

result
_Pool_Destroy(
    pool_hnd        poolHnd_In)
{
    if(!l_PoolHead || poolHnd_In > l_PoolHead->hnd) 
        return (!l_PoolHead) ? POOL_NOPOOL : POOL_INVPLHND;
    pool* target = l_PoolHead;
    pool* next = l_PoolHead;
    while(poolHnd_In != target->hnd)
    {
        target = target->pPrev;
        next = (poolHnd_In != target->hnd) ? next->pPrev : next;
    }
    if(target->state == POOL_STATE_FREED) 
        return POOL_DBLFREE;
    void* voids = target->voids;
    MEM_ASSERT(Pool_Free(voids));
    MEM_ASSERT(Pool_Realloc(sizeof(pool), target));
    next->pPrev = (next != target) ? target : null;
    target->state = POOL_STATE_FREED;
    return POOL_SUCCESS;
};

result
_Pool_Kill(void)
{
    Pool_Free_All();
    l_PoolHead = null;
    return POOL_SUCCESS;
};

result 
_Pool_Resize(
    pool_hnd        poolHnd_In,
    uint64          size_In)
{
    if(!l_PoolHead || poolHnd_In > l_PoolHead->hnd) 
        return (!l_PoolHead) ? POOL_NOPOOL : POOL_INVPLHND;
    pool* target = l_PoolHead;
    pool* next = l_PoolHead;
    while(poolHnd_In != target->hnd)
    {
        target = target->pPrev;
        next = (poolHnd_In != target->hnd) ? next->pPrev : next;
    }
    if(target->state & POOL_STATE_FREED || target->state & POOL_STATE_STATIC) 
        return (target->state & POOL_STATE_FREED) ? POOL_NOACTIVE : POOL_NORESIZE;

    size_In += POOL_SECTION_SIZE - (size_In % POOL_SECTION_SIZE);
    MEM_ASSERT(Pool_Realloc(sizeof(pool) + size_In, target));
    next->pPrev = target;
    target->oEnd = size_In / POOL_SECTION_SIZE;
    return POOL_SUCCESS;
};

result
_Pool_Reserve(
    uint64          size_In,
    res_hnd*        resHnd_Out)
{
    if(!size_In || !resHnd_Out)
        return (!size_In) ? POOL_NOSIZE : POOL_NOOUTPUT;
    if(!l_PoolHead)
        return POOL_NOPOOL;
    
    size_In += RES_META_SIZE;
    size_In += POOL_SECTION_SIZE - (size_In % POOL_SECTION_SIZE);
    uint32 sizeAlign = size_In / sizeof(uint32);
    size_In /= POOL_SECTION_SIZE;

    pool* target = l_PoolHead;
    uint32 offset = 0;
    while(target && !offset)
    {
        for(uint32 i = 0; i < target->voids[1] && !offset; i++)
        {
            uint32* voidRes = (void*)target + (target->voids[2 + i] * POOL_SECTION_SIZE);
            uint32 voidSize = voidRes[1];
            offset = (size_In <= voidSize) ? l_pool_reclaim(target, voidRes, i, size_In) : 0;
        }
        offset = (!offset) ? l_pool_reserve(target, size_In) : offset;
        target = (!offset) ? target->pPrev : target;
    }
    if(!offset)
        return POOL_NOSPACE;
        
    *resHnd_Out = 0;
    RES_HND_ASSEMBLE(offset, target->hnd, *resHnd_Out);
    return POOL_SUCCESS;
};

result
_Pool_Reserve_At(
    uint64          size_In, 
    pool_hnd        poolHnd_In, 
    res_hnd*        resHnd_Out)
{
    if(!size_In || !resHnd_Out)
        return (!size_In) ? POOL_NOSIZE : POOL_NOOUTPUT;
    if(!l_PoolHead)
        return POOL_NOPOOL;
    
    size_In += RES_META_SIZE;
    size_In += POOL_SECTION_SIZE - (size_In % POOL_SECTION_SIZE);
    uint32 sizeAlign = size_In / sizeof(uint32);
    size_In /= POOL_SECTION_SIZE;

    pool*target = l_PoolHead;
    while(target->hnd != poolHnd_In)
        target = target->pPrev;
    if(!target)
        return POOL_INVPLHND;

    uint32 offset = 0;
    for(uint32 i = 0; i < target->voids[1] && !offset; i++)
    {
        uint32* voidRes = (void*)target + (target->voids[2 + i] * POOL_SECTION_SIZE);
        uint32 voidSize = voidRes[1];
        offset = (size_In <= voidSize) ? l_pool_reclaim(target, voidRes, i, size_In) : 0;
    }
    offset = (!offset) ? l_pool_reserve(target, size_In) : offset;
    if(!offset)
        return POOL_NOSPACE;

    *resHnd_Out = 0;
    RES_HND_ASSEMBLE(offset, target->hnd, *resHnd_Out);
    return POOL_SUCCESS;
}

result
_Pool_Release(
    res_hnd         resHnd_In)
{
    uint32* res = null;
    uint32 offset = 0;
    pool* target = null;
    result resCheck = l_pool_obtainRes(resHnd_In, &res, &offset, &target);
    if(resCheck != POOL_SUCCESS)
        return resCheck;
        
    if(res[0] == RES_STATUS_FREE)
        return POOL_DBLFREE;

    target->resCount--;
    uint32 size = RES_SIZE_SECTOALIGN(res[1]);
    uint32 addOffset = offset;
    uint32 removeOffsets[] = {0, 0, 0};
    // Coalesce front.
    if(res[size] == RES_STATUS_FREE)
    {
        res[1] += res[size + 1];
        removeOffsets[0] = offset + res[size + 1];
    }
    // if(res[size] == RES_STATUS_FREE)
    // {
    //     res[1] += res[size + 1];
    //     uint32 nextOffset = offset + res[size + 1];
    //     l_pool_removeVoidEntry(target, nextOffset);
    // }
    // Coalesce back.
    if(offset > 0 && res[-1] == RES_STATUS_FREE)
    {
        uint32 prevSize = RES_SIZE_SECTOALIGN(res[-2]);
        res -= prevSize;
        offset -= res[1];
        res[1] += res[prevSize + 1];
        addOffset = 0;
    }
    // if(offset > 0 && res[-1] == RES_STATUS_FREE)
    // {
    //     offset -= res[-2];
    //     uint32 prevSize = RES_SIZE_SECTOALIGN(res[-2]);
    //     *(res - (prevSize - 1)) += res[1];
    //     if(offset + *(res - (prevSize - 1)) >= target->oData)
    //     {
    //         target->oData = offset;
    //         l_pool_removeVoidEntry(target, offset);
    //         return POOL_SUCCESS;
    //     }
    //     offset = 0;
    // }
    // Pop back target->oData if res is most recent.
    if(offset + res[1] >= target->oData)
    {
        res[0] = res[1] = 0;
        target->oData = offset;
        removeOffsets[(removeOffsets[0] > 0)] = offset;
        addOffset = 0;
    }
    l_pool_setResMeta(res, RES_STATUS_FREE, res[1]);
    for(uint32 i = 0; removeOffsets[i]; i++)
        l_pool_removeVoidEntry(target, removeOffsets[i]);
    if(addOffset)
    {
        if(target->voids[1] >= target->voids[0])
        {
            void* voids = target->voids;
            Pool_Realloc((target->voids[0] * 2) + 2, voids);
            target->voids = voids;
            target->voids[0] *= 2;
        }
        target->voids[2 + target->voids[1]] = offset;
        target->voids[1]++;
    }
    return POOL_SUCCESS;
};

result
_Pool_Retrieve(
    res_hnd         resHnd_In, 
    void**          rawPtr_Out)
{
    uint32* res = null;
    result resCheck = l_pool_obtainRes(resHnd_In, &res, null, null);
    if(resCheck != POOL_SUCCESS)
        return resCheck;

    if(res[0] == RES_STATUS_FREE)
        return POOL_RESFREE;

    *rawPtr_Out = (void*)(res + 2);
    return POOL_SUCCESS;
};

result
_Pool_Write(
    res_hnd         resHnd_In, 
    void*           data_In, 
    uint64          dataSize_In, 
    uint64          offset_In)
{
    uint32* res = null;
    result resCheck = l_pool_obtainRes(resHnd_In, &res, null, null);
    if(resCheck != POOL_SUCCESS)
        return resCheck;

    if(res[0] == RES_STATUS_FREE)
        return POOL_RESFREE;
    
    uint64 resSize = (res[1] * POOL_SECTION_SIZE) - RES_META_SIZE;
    if(offset_In >= resSize)
        return POOL_INVOFFST;
    uint64 lim = (dataSize_In) ? (offset_In + dataSize_In) : resSize;
    result msgFit = POOL_SUCCESS;
    if(lim > resSize)
    {
        lim = resSize;
        msgFit = POOL_RESNOFIT;
    }
    uint8* src = (uint8*)data_In;
    uint8* dest = (uint8*)(res + 2);
    for(uint64 i = offset_In; i < lim; i++, src++)
        *(dest + i) = *src;

    return msgFit;
}

result
_Pool_Modify(
    res_hnd*        resHnd_InOut, 
    uint64          size_In)
{
    uint32* res = null;
    pool* target = null;
    result resCheck = l_pool_obtainRes(*resHnd_InOut, &res, null, &target);
    if(resCheck != POOL_SUCCESS)
        return resCheck;

    res_hnd newResHnd = 0;
    result checkAll = _Pool_Reserve_At(size_In, target->hnd, &newResHnd);
    if(checkAll != POOL_SUCCESS)
        return checkAll;

    uint64 range = res[1] * POOL_SECTION_SIZE;
    checkAll = _Pool_Write(newResHnd, (void*)(res + 2), range, 0);
    if(checkAll != POOL_SUCCESS)
        return checkAll;

    checkAll = _Pool_Release(*resHnd_InOut);
    if(checkAll != POOL_SUCCESS)
        return checkAll;

    *resHnd_InOut = newResHnd;
    return POOL_SUCCESS;
};

#endif // MEM_POOL_H