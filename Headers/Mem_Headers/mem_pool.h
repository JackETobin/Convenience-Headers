// TODO: Write the documentation including the possible define declarations.
// TODO: Hook up a res_clear function that clears released data in debug mode -> clear to magic number.
/*
 * USE_MEM_DEBUG
 * USE_MEM_ALLOC
 * USE_MEM_ALL
 */
#ifndef MEM_POOL_H
#define MEM_POOL_H

#define POOL_ALIGN                      8
#define POOL_SECTION_SIZE               64
#define RES_MIN_SIZE                    3
#define POOL_VOID_DEFAULT               12

#define POOL_STATE_READY                (uint16)0X01
#define POOL_STATE_FREED                (uint16)0X02
#define POOL_STATE_STATIC               (uint16)0X04
#define POOL_STATE_RESIZE               (uint16)0X08
#define POOL_STATE_DYNAMIC              (uint16)0X10

#define RES_STATUS_FAULT                (uint32)0X00
#define RES_STATUS_ACTIVE               (uint32)0X01
#define RES_STATUS_FREE                 (uint32)0X02

#define RES_HND_ASSEMBLE(r, p, hnd)     (hnd |= (r << 16) | p)
#define RES_HND_OFFSET(r, hnd)          (r = (hnd >> 16))
#define RES_HND_POOL(p, hnd)            (p = (uint16)hnd)

#define RES_SIZE_ALIGNTOSEC(align)      (align * sizeof(uint32)) / POOL_SECTION_SIZE
#define RES_SIZE_SECTOALIGN(sec)        (sec * POOL_SECTION_SIZE) / sizeof(uint32)

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

    // #define packed          __attribute__((__packed__))

    #define unused(x)       (void)(x)

    #define POOL_SUCCESS    (result)0X0A // memory manager success.
    #define POOL_NOSPACE    (result)0X0B // Not enough space available.
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
    #define POOL_RESIZE     (result)0X1B // Memory manager critical failure.
#endif // MEM_DEFINES_H

#if defined(USE_MEM_DEBUG) || defined(USE_MEM_ALL)
    #include "mem_debug.h"
    #define DBG(func) _Debug_Catch(func, #func, line, file)
#else
    #define DBG(func) func
#endif // USE_MEM_DEBUG

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

#if defined(USE_MEM_DEBUG) || defined(USE_MEM_ALL)
    #define RES_META_SIZE                   (4 * sizeof(uint32))
    #define DEB_MAGNUM                      0XCDCDCDCDU
    #define RES_MAGNUM                      0
    #define RES_STATUS                      1
    #define RES_SIZE                        2
    #define RES_DATA                        3
#else
    #define RES_META_SIZE                   (2 * sizeof(uint32))
    #define RES_STATUS                      0
    #define RES_SIZE                        1
    #define RES_DATA                        2
#endif // USE_MEM_DEBUG

typedef uint16      pool_hnd;
typedef uint64      res_hnd;

typedef enum {
    POOL_STATIC     = (uint16)POOL_STATE_STATIC,
    POOL_RESIZABLE  = (uint16)POOL_STATE_RESIZE,
    POOL_DYNAMIC    = (uint16)POOL_STATE_DYNAMIC
} resize_type;

typedef struct Void_Entry_St
{
    uint32          size;
    uint32          offset;
} rel_entry;

typedef struct Pool_St
{
    struct Pool_St* pPrev;
    rel_entry*      released;
    uint32          resCount;
    uint32          relCount;
    uint32          relMax;
    uint32          oEnd;
    uint32          oData;
    uint16          state;
    pool_hnd        hnd;
} pool;

persist pool* l_PoolHead = null;

#if !defined(MEM_ALLOC_H)
    internal result
    l_pool_freeAll()
    {
        pool* target = l_PoolHead;
        while(target)
        {
            pool* toFree = target;
            target = target->pPrev;

            Pool_Free(toFree->released);
            Pool_Free(toFree);
        }
        l_PoolHead = null;
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
l_pool_setResMeta(
    uint32*         res_In, 
    uint32          status_In, 
    uint32          size_In)
{
    uint64 sizeAlign = RES_SIZE_SECTOALIGN(size_In);
    if(!sizeAlign)
        return sizeAlign;
#if defined(USE_MEM_DEBUG) || defined(USE_MEM_ALL)
    res_In[RES_MAGNUM] = res_In[sizeAlign - 1] = DEB_MAGNUM;
#endif
    res_In[RES_STATUS] = status_In;
    res_In[RES_SIZE] = size_In;
    return sizeAlign;
}

internal uint32
l_pool_reclaim(
    pool*           target_In, 
    uint32          adjustedSize_In)
{
    for(uint32 i = 0; i < target_In->relCount; i++)
        if(target_In->released[i].size >= adjustedSize_In)
        {
            if(target_In->released[i].size > adjustedSize_In + RES_MIN_SIZE)
            {
                target_In->released[i].size -= adjustedSize_In;
                return target_In->released[i].offset + target_In->released[i].size;
            }
            return target_In->released[i].offset;
        }
    return 0;
}

internal uint32
l_pool_new(pool* target_In, uint32 adjustedSize_In)
{
    uint32 offset = 0;
    if(adjustedSize_In <= (target_In->oEnd - target_In->oData))
    {
        offset = target_In->oData;
        target_In->oData += adjustedSize_In;
        target_In->resCount++;
    }
    return offset;
}

internal uint32
l_pool_reserve(
    pool*           target_In, 
    uint32          adjustedSize_In)
{
    uint32 offset = l_pool_reclaim(target_In, adjustedSize_In);
    offset = (!offset) ? l_pool_new(target_In, adjustedSize_In) : offset;
    if(offset)
    {
        uint32* res = (void*)target_In + (offset * POOL_SECTION_SIZE);
        l_pool_setResMeta(res, RES_STATUS_ACTIVE, adjustedSize_In);
    }
    return offset;
}

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
    resize_type     resize_In,
    pool_hnd*       poolHnd_Out)
{
#ifndef MEM_ALLOC_H
    unused(tag_In);
#endif // MEM_ALLOC_H
    size_In += (2 * POOL_SECTION_SIZE) - (size_In % POOL_SECTION_SIZE);
    pool* new = null;
    rel_entry* bucket = null;
    DBG(Pool_Alloc(size_In, tag_In, new));
    DBG(Pool_Alloc((sizeof(*bucket) * POOL_VOID_DEFAULT), tag_In, bucket));
    if(!new || !bucket) 
    {
        if(new) Pool_Free(new);
        if(bucket) Pool_Free(bucket);
        return POOL_NOBUILD;
    }
    uint32 end = size_In / POOL_SECTION_SIZE;
    uint8 hnd = (l_PoolHead) ? l_PoolHead->hnd + 1 : 0;
    uint8 state = POOL_STATE_READY | resize_In;
    *new = (pool){l_PoolHead, bucket, 0, 0, POOL_VOID_DEFAULT, end, 1, state, hnd};

    l_PoolHead = new;
    *poolHnd_Out = new->hnd;
    return POOL_SUCCESS;
};

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
    DBG(Pool_Free(target->released));
    DBG(Pool_Realloc(sizeof(pool), target));
    next->pPrev = (next != target) ? target : null;
    target->state = POOL_STATE_FREED;
    return POOL_SUCCESS;
};

/* 
 * FUNCTION: Frees all allocations associated with mem_pool.h
 *  - no input -
 * 
 * Nullifies the linked list used internally to access allocations.
 */
result
_Pool_Kill(void)
{
    Pool_Free_All();
    l_PoolHead = null;
    return POOL_SUCCESS;
};

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
    uint64          size_In)
{
    if(!l_PoolHead || poolHnd_In > l_PoolHead->hnd) 
        return (!l_PoolHead) ? POOL_NOPOOL : POOL_INVPLHND;
    pool* top = l_PoolHead;
    pool* next = l_PoolHead;
    while(poolHnd_In != l_PoolHead->hnd)
    {
        l_PoolHead = l_PoolHead->pPrev;
        next = (poolHnd_In != l_PoolHead->hnd) ? next->pPrev : next;
    }
    uint16 state = l_PoolHead->state;
    if(state & POOL_STATE_FREED || state & POOL_STATE_STATIC) 
        return (state & POOL_STATE_FREED) ? POOL_NOACTIVE : POOL_NORESIZE;

    size_In += POOL_SECTION_SIZE - (size_In % POOL_SECTION_SIZE);
    DBG(Pool_Realloc(sizeof(pool) + size_In, l_PoolHead));
    next->pPrev = (next->hnd == l_PoolHead->hnd) ? next->pPrev : l_PoolHead;
    l_PoolHead->oEnd = size_In / POOL_SECTION_SIZE;
    l_PoolHead = (l_PoolHead->hnd == top->hnd) ? l_PoolHead : top;
    return POOL_SUCCESS;
};

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
    res_hnd*        resHnd_Out)
{
    if(!size_In || !resHnd_Out)
        return (!size_In) ? POOL_NOSIZE : POOL_NOOUTPUT;
    if(!l_PoolHead)
        return POOL_NOPOOL;
    
    size_In += RES_META_SIZE;
    size_In += (POOL_SECTION_SIZE - (size_In % POOL_SECTION_SIZE));
    size_In /= POOL_SECTION_SIZE;

    pool* target = l_PoolHead;
    uint32 offset = 0;
    while(target && !offset)
    {
        offset = l_pool_reserve(target, size_In);
        if(!offset && (target->state & POOL_STATE_DYNAMIC))
        {
            pool_hnd hnd = target->hnd;
            uint64 poolSize = target->oEnd * POOL_SECTION_SIZE;
            DBG(_Pool_Resize(hnd, (poolSize * 1.5) + (size_In * POOL_SECTION_SIZE)));
            target = l_PoolHead;
            while(target->hnd != hnd && target)
                target = target->pPrev;
            offset = l_pool_reserve(target, size_In);
        }
        target = (!offset) ? target->pPrev : target;
    }
    if(!offset)
        return POOL_NOSPACE;

    *resHnd_Out = 0;
    RES_HND_ASSEMBLE(offset, target->hnd, *resHnd_Out);
    return POOL_SUCCESS;
};

/*
 * FUNCTION: Reserves space in a memory pool.
 *  - input  - size_In:     bytesize of the space to be reserved.
 *  - input  - poolHnd_In:  handle of the pool in which a reservation is to be made.
 *  - output - resHnd_Out:  buffer to be filled with a handle corresponding the the newly reserved 
 *                          memory.
 * 
 * Reservations are made in a designated pool. This function will not attempt to find space in 
 * another location should the designated pool be full. If the pool is flagged as dynamic, the pool 
 * will resize, then reserve space.
 */
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
    size_In /= POOL_SECTION_SIZE;

    pool*target = l_PoolHead;
    while(target->hnd != poolHnd_In)
        target = target->pPrev;
    if(!target)
        return POOL_INVPLHND;

    uint32 offset = l_pool_reserve(target, size_In);
    result resizeFlag = 0;
    if(!offset && (target->state & POOL_STATE_DYNAMIC))
    {
        pool_hnd hnd = target->hnd;
        uint64 newSize = (uint64)(target->oEnd * 1.75 + size_In) * POOL_SECTION_SIZE;
        resizeFlag = DBG(_Pool_Resize(hnd, newSize));
        target = l_PoolHead;
        while(target->hnd != hnd && target)
            target = target->pPrev;
        offset = l_pool_reserve(target, size_In);
    }
    if(!offset)
        return POOL_NOSPACE;

    *resHnd_Out = 0;
    RES_HND_ASSEMBLE(offset, target->hnd, *resHnd_Out);
    return (resizeFlag) ? POOL_RESIZE : POOL_SUCCESS;
}

/* 
 * FUNCTION: Releases a reservation, allowing for reallocation and preventing further writing.
 *  - input  - resHnd_In: a handle to the designated reservation intended to be released.
 *
 * Data within the reservation is not destroyed, the reservation is simply flagged as released and
 * if possible, coalesced into adjacent released reservations. Handle is then zero'd out so that it
 * cannot be used for an operatioon other than another call to _Pool_Reserve().
 */
result
_Pool_Release(
    res_hnd*        resHnd_InOut)
{
    uint32* res = null;
    uint32 offset = 0;
    pool* target = null;
    result resCheck = l_pool_obtainRes(*resHnd_InOut, &res, &offset, &target);
    if(resCheck != POOL_SUCCESS)
        return resCheck;
    
    if(res[RES_STATUS] == RES_STATUS_FREE)
        return POOL_DBLFREE;

    rel_entry released = { res[RES_SIZE], offset};
    rel_entry* relList = target->released;
    uint32 relIndex = target->relCount;
    while(relIndex)
    {
        if(relList[relIndex - 1].offset < offset)
        {
            relList[relIndex] = released;
            break;
        }
        relList[relIndex] = (relIndex != 0) ? relList[relIndex - 1] : released;
        relIndex--;
    }
    relList[relIndex] = (relIndex) ? relList[relIndex] : released;
    target->relCount++;

    uint32 coalesce[] = { 0, 0, 0};
    for(uint32 i = relIndex - 1; i < relIndex + 1; i++)
        if(relList[i].offset + relList[i].size == relList[i + 1].offset)
        {
            relList[i].size += relList[i + 1].size;
            coalesce[0]++;
            coalesce[coalesce[0]] = i + 1;
        }
    while(coalesce[0])
    {
        for(uint32 i = coalesce[coalesce[0]]; i < target->relCount; i++)
            relList[i] = relList[i + 1];
        target->relCount--;
        coalesce[0]--;
    }
    if(relList[target->relCount - 1].offset + relList[target->relCount - 1].size == target->oData)
    {
        target->oData -= relList[target->relCount - 1].size;
        relList[target->relCount - 1] = (rel_entry){ 0 };
        target->relCount--;
    }
    target->resCount--;
    *resHnd_InOut = 0;
    return POOL_SUCCESS;
}

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

// TODO: Set up debug on the status check. Leave the status unchecked in release, live on the edge.
// TODO: Move the nuts and bolts of the data write to an internal function that we can call without the checks.
/* 
 * FUNCTION: Copies data into a pool reservation.
 *  - input  - resHnd_In: a handle to the reservation designated to be written to.
 *  - input  - data_In: a pointer to the source data to be written.
 *  - input  - dataSize_In: the size of the source data to be written.
 *  - input  - offest_In: the offset into the destination reservation where data is to be written.
 * 
 * This function prevents overwriting the reservation. If dataSize_In is zero, the function will 
 * write to the end of the reservation. It will read past the source data buffer should the buffer 
 * be smaller than the reservation.
 */
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

    if(res[RES_STATUS] == RES_STATUS_FREE)
        return POOL_RESFREE;
    
    uint64 resSize = (res[RES_SIZE] * POOL_SECTION_SIZE) - RES_META_SIZE;
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
    uint8* dest = (uint8*)(res + RES_DATA);
    for(uint64 i = offset_In; i < lim; i++, src++)
        *(dest + i) = *src;

    return msgFit;
}

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
    uint64          size_In)
{
    uint32* res = null;
    pool* target = null;
    result checkAll = l_pool_obtainRes(*resHnd_InOut, &res, null, &target);
    if(checkAll != POOL_SUCCESS)
        return checkAll;

    res_hnd newResHnd = 0;
    checkAll = _Pool_Reserve_At(size_In, target->hnd, &newResHnd);
    if(checkAll == POOL_RESIZE)
    {
        pool_hnd hnd = target->hnd;
        for(target = l_PoolHead; target->hnd != hnd; target = target->pPrev);
        checkAll = l_pool_obtainRes(*resHnd_InOut, &res, null, &target);
    }
    if(checkAll != POOL_SUCCESS)
        return checkAll;

    uint64 range = res[RES_SIZE] * POOL_SECTION_SIZE;
    checkAll = _Pool_Write(newResHnd, (void*)(res + RES_DATA), range, 0);
    if(checkAll != POOL_SUCCESS)
        return checkAll;

    checkAll = _Pool_Release(resHnd_InOut);
    if(checkAll != POOL_SUCCESS)
        return checkAll;

    *resHnd_InOut = newResHnd;
    return checkAll;
};

#if !defined(MEM_HEADERS_H)

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

#endif // MEM_POOL_H