// TODO: Write the documentation including the possible define declarations.
// TODO: Hook up a res_clear function that clears released data in debug mode -> clear to magic number.
/*
 * USE_MEM_DEBUG
 * USE_MEM_ALLOC
 * USE_MEM_ALL
 */
#ifndef MEM_POOL_H_OLD
#define MEM_POOL_H_OLD


#if !defined(MEM_HEADERS_H) || !defined(PLATFORM_H)
    #define null                ((void*)0)
    #define true                (uint8)0X01
    #define false               (uint8)0X00

    typedef unsigned char       uint8;
    typedef unsigned short      uint16;
    typedef unsigned int        uint32;
    typedef unsigned long long  uint64;

    typedef double              float64;
    typedef uint8               result;
#endif // MEM_TYPES
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

#if !defined(DEFINES_H)
    #define persist         static
    #define internal        static

    #define unused(x)       (void)(x)
#endif // DEFINES_H

#define POOL_SUCCESS        (result)0X02 // memory pool success.
#define POOL_NOSPACE        (result)0X19 // Not enough space available.
#define POOL_NOPOOL         (result)0X1A // No avaolable pools, call Pool_Build().
#define POOL_NOSIZE         (result)0X1B // No size provided.
#define POOL_NOALLOC        (result)0X1C // No allocator provided.
#define POOL_NOFREE         (result)0X1D // No free provided.
#define POOL_NOBUILD        (result)0X1E // Unable to allocate space.
#define POOL_NOACTIVE       (result)0X1F // Pool has been freed already.
#define POOL_NORESIZE       (result)0X20 // Pool is not resizeable.
#define POOL_NOOUTPUT       (result)0X21 // No output buffer provided.
#define POOL_INVPLHND       (result)0X22 // Invalid pool handle.
#define POOL_INVREHND       (result)0X23 // Invalid reservation handle.
#define POOL_RESFREE        (result)0X24 // Reservation has been released.
#define POOL_DBLFREE        (result)0X25 // Release has already been called on this reservation.
#define POOL_RESNOFIT       (result)0X26 // Reservatioin isn't large enough, data has been truncated.
#define POOL_INVOFFST       (result)0X27 // Invalid buffer offset on write attempt.
#define POOL_RESFAULT       (result)0x28 // Unable to obtain the reservation.
#define POOL_RESIZE         (result)0X29 // Pool has been resized and a raw pointer update might be necessary.

#if defined(MODE_DEBUG)
    #if !defined(MEM_HEADERS_H)
        #include "../log.h"
        #define DBG(func) _Debug_Catch(func, #func, line, file)
    #endif // MEM_HEADERS_H
    #define RES_META_SIZE                   (4 * sizeof(uint32))
    #define DEB_MAGNUM                      0XCDCDCDCDU
    #define RES_MAGNUM                      0
    #define RES_STATUS                      1
    #define RES_SIZE                        2
    #define RES_DATA                        3
#else
    #if !defined(MEM_HEADERS_H)
        #define DBG(func) func
    #endif // MEM_HEADERS_H
    #define RES_META_SIZE                   (2 * sizeof(uint32))
    #define RES_STATUS                      0
    #define RES_SIZE                        1
    #define RES_DATA                        2
#endif // MODE_DEBUG

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

#if defined(USE_MEM_ALLOC) || defined (USE_MEM_ALL)
    #include "mem_alloc.h"
    internal result 
    l_pool_alloc(
        uint64*     size_InOut, 
        uint32      align_In, 
        char*       tag_In, 
        void**      mem_Out)
    { return _Mem_Alloc_Allign(size_InOut, align_In, tag_In, (p_mem)mem_Out); }
    #define pool_realloc(size_InOut, mem_InOut)                             \
        _Mem_Realloc((uint64*)&size_InOut, (p_mem)&mem_InOut)
    #define Pool_Free(mem_In)                                               \
        _Mem_Free((p_mem)&mem_In)
    #define Pool_Free_All()                                                 \
        _Mem_Kill();
#else
    #include <stdlib.h>
    internal result
    l_pool_alloc(
        uint64*     size_InOut, 
        uint32      align_In, 
        char*       tag_In, 
        void**      mem_Out)
    {
        void* new = malloc(size_In);
        if(new)
        {
            *mem_Out = new;
            return POOL_SUCCESS;
        }
        return POOL_NOALLOC;
    }
    #define Pool_Alloc(size_In, tag_In, mem_Out) l_pool_alloc(size_In, (void**)&mem_Out)

    internal result
    l_pool_realloc(uint64 size_In, void** mem_InOut)
    {
        void* new = realloc(*mem_InOut, size_In);
        if(new)
        {
            *mem_InOut = new;
            return POOL_SUCCESS;
        }
        return POOL_NORESIZE;
    }
    #define Pool_Realloc(size_In, mem_InOut)     l_pool_realloc(size_In, (void*)&mem_InOut)

    internal result
    l_pool_free(void** mem_In)
    {
        free(mem_In);
        *mem_In = null;
        return POOL_SUCCESS;
    }
    #define Pool_Free(mem_In)                    l_pool_free((void*)&mem_In)
#endif // MEM_ALLOC_H || CUSTOM_ALLOCATOR

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
    // TODO: Strip on debug.
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

// internal result
// l_pool_write(uint8* src_In, uint8* dst_In, uint64 range_In)
// {
//     for(uint8* lim = src_In + range_In; src_In < lim; src_In++, dst_In++)
//         *dst_In = *src_In;
//     return POOL_SUCCESS;
// }

internal uint32
l_pool_reclaim(
    pool*           target_In, 
    uint32          adjustedSize_In)
{
    uint32 offset = 0;
    uint32 decrement = 0;
    rel_entry* rel = target_In->released;
    for(uint32 i = 0; i < target_In->relCount && !offset; i++)
        if(rel[i].size >= adjustedSize_In)
        {
            if(rel[i].size > adjustedSize_In + RES_MIN_SIZE)
            {
                rel[i].size -= adjustedSize_In;
                offset = rel[i].offset + rel[i].size;
            }
            decrement += (!offset);
            offset = (!offset) ? rel[i].offset : offset;
        }
    if(offset)
    {
        uint32* res = (void*)target_In + (offset * POOL_SECTION_SIZE);
        l_pool_setResMeta(res, RES_STATUS_ACTIVE, adjustedSize_In);
        target_In->resCount++;
    }
    target_In->relCount -= decrement;
    return offset;
}

// internal uint32
// l_pool_new(pool* target_In, uint32 adjustedSize_In)
// {
//     uint32 offset = target_In->oData;
//     target_In->oData += adjustedSize_In;
//     return offset;
// }

internal uint32
l_pool_reserve(
    pool*           target_In, 
    uint32          adjustedSize_In)
{
    // uint32 offset = l_pool_reclaim(target_In, adjustedSize_In);
    // if(!offset)
    // {
    uint32 offset = target_In->oData;
    target_In->oData += adjustedSize_In;
    // }
    uint32* res = (void*)target_In + (offset * POOL_SECTION_SIZE);
    l_pool_setResMeta(res, RES_STATUS_ACTIVE, adjustedSize_In);
    target_In->resCount++;
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
    uint64 bucketSize = sizeof(*bucket) * POOL_VOID_DEFAULT;
    DBG(Pool_Alloc(bucketSize, tag_In, bucket));
    if(!new || !bucket) 
    {
        if(new) Pool_Free(new);
        if(bucket) Pool_Free(bucket);
        return POOL_NOBUILD;
    }
    uint32 end = size_In / POOL_SECTION_SIZE;
    uint8 hnd = (l_PoolHead) ? l_PoolHead->hnd + 1 : 0;
    uint8 state = POOL_STATE_READY | resize_In;
    *new = (pool){l_PoolHead, bucket, 0, 0, bucketSize / sizeof(*bucket), end, 1, state, hnd};

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
    l_PoolHead = (target == l_PoolHead) ? l_PoolHead->pPrev : l_PoolHead;
    next->pPrev = target->pPrev;
    // if(target->state == POOL_STATE_FREED) 
    //     return POOL_DBLFREE;
    DBG(Pool_Free(target->released));
    DBG(Pool_Free(target));
    // target->state = POOL_STATE_FREED;
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

    size_In += sizeof(pool);
    size_In += POOL_SECTION_SIZE - (size_In % POOL_SECTION_SIZE);
    DBG(Pool_Realloc(size_In, l_PoolHead));
    next->pPrev = (next->hnd == l_PoolHead->hnd) ? next->pPrev : l_PoolHead;
    l_PoolHead->oEnd = size_In / POOL_SECTION_SIZE;
    l_PoolHead = (l_PoolHead->hnd == top->hnd) ? l_PoolHead : top;
    return POOL_SUCCESS;
};

internal result
l_pool_sizeCheck(
    pool**          target_In,
    uint64          adjustedSize_In)
{
    pool* target = *target_In;
    result check = (adjustedSize_In > (target->oEnd - target->oData)) ? POOL_NOSPACE : POOL_SUCCESS;
    if(check == POOL_NOSPACE && target->state & POOL_STATE_DYNAMIC)
    {
        pool_hnd hnd = target->hnd;
        uint64 newSize = (uint64)(target->oEnd * 1.75 + adjustedSize_In) * POOL_SECTION_SIZE;
        DBG(_Pool_Resize(hnd, newSize));
        target = l_PoolHead;
        while(target->hnd != hnd && target)
            target = target->pPrev;
        *target_In = target;
        check = POOL_SUCCESS;
    }
    return check;
}

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
        offset = l_pool_reclaim(target, size_In);
        if(!offset)
        {
            if(l_pool_sizeCheck(&target, size_In) == POOL_SUCCESS)
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

    uint32 offset = l_pool_reclaim(target, size_In);
    if(!offset)
    {
        if(l_pool_sizeCheck(&target, size_In) != POOL_SUCCESS)
            return POOL_NOSPACE;
        offset = l_pool_reserve(target, size_In);
    }

    *resHnd_Out = 0;
    RES_HND_ASSEMBLE(offset, target->hnd, *resHnd_Out);
    return POOL_SUCCESS;
}

internal result
l_pool_release(
    pool*           target_In, 
    uint32*         toFree_In, 
    uint32          offset_In)
{
    target_In->resCount--;
    rel_entry* rel = target_In->released;
    uint32 count = target_In->relCount;
    uint32 iFree = count;
    while(iFree && rel[iFree - 1].offset > offset_In)
        iFree--;
    
    rel_entry new = { toFree_In[RES_SIZE], offset_In };
    rel_entry prev = (iFree > 0) ? rel[iFree - 1] : (rel_entry){ 0 };
    if((prev.offset + prev.size) == new.offset)
    {
        rel[iFree - 1] = (rel_entry){ prev.size + new.size, prev.offset };
        iFree--;
    } 
    else 
    {
        for(uint32 i = count; i > iFree; i--)
            rel[i] = rel[i - 1];
        rel[iFree] = new;
        target_In->relCount++;
    }
    rel_entry next = (iFree < count) ? rel[iFree] : (rel_entry){ 0 };
    if(rel[iFree].size + rel[iFree].offset == next.offset)
    {
        rel[iFree].size += next.size;
        for(uint32 i = iFree + 1; i < count; i++)
            rel[i] = rel[i + 1];
        target_In->relCount--;
    }
    if(rel[iFree].size + rel[iFree].offset == target_In->oData)
        target_In->oData -= rel[iFree].size;
    
    return POOL_SUCCESS;
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
    
    return l_pool_release(target, res, offset);
    // rel_entry released = { res[RES_SIZE], offset};
    // rel_entry* relList = target->released;
    // uint32 relIndex = target->relCount;
    // while(relIndex)
    // {
    //     if(relList[relIndex - 1].offset < offset)
    //     {
    //         relList[relIndex] = released;
    //         break;
    //     }
    //     relList[relIndex] = (relIndex != 0) ? relList[relIndex - 1] : released;
    //     relIndex--;
    // }
    // relList[relIndex] = (relIndex) ? relList[relIndex] : released;
    // target->relCount++;

    // for(uint32 i = relIndex + (relIndex > (target->relCount - 1)); i; i--)
    //     if(relList[i - 1].offset + relList[i - 1].size == relList[i].offset)
    //     {
    //         relList[i - 1].size += relList[i].size;
    //         for(uint32 j = i; j < target->relCount; j++)
    //             relList[j] = relList[j + 1];
    //         target->relCount--;
    //     }
    // if(relList[target->relCount - 1].offset + relList[target->relCount - 1].size == target->oData)
    // {
    //     target->oData -= relList[target->relCount - 1].size;
    //     target->relCount--;
    // }
    // target->resCount--;
    // *resHnd_InOut = 0;
    // return POOL_SUCCESS;
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

    if(res[RES_STATUS] == RES_STATUS_FREE)
        return POOL_RESFREE;

    *rawPtr_Out = (void*)(res + RES_DATA);
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
    size_In += RES_META_SIZE;
    size_In += POOL_SECTION_SIZE - (size_In % POOL_SECTION_SIZE);
    size_In /= POOL_SECTION_SIZE;

    uint32* res = null;
    pool* target = null;
    result checkAll = l_pool_obtainRes(*resHnd_InOut, &res, null, &target);
    if(checkAll != POOL_SUCCESS)
        return checkAll;

    uint32 offset = l_pool_reclaim(target, size_In);
    if(!offset)
    {
        if(l_pool_sizeCheck(&target, size_In) != POOL_SUCCESS)
            return POOL_NOSPACE;
        offset = l_pool_reserve(target, size_In);
    }

    res_hnd newResHnd = 0;
    RES_HND_ASSEMBLE(offset, target->hnd, newResHnd);

    uint64 range = (res[RES_SIZE] * POOL_SECTION_SIZE) - RES_META_SIZE;
    // TODO: Split write into an internal function and an external wrapper.
    checkAll = _Pool_Write(newResHnd, (void*)(res + RES_DATA), range, 0);
    if(checkAll != POOL_SUCCESS)
        return checkAll;

    checkAll = _Pool_Release(resHnd_InOut);
    if(checkAll != POOL_SUCCESS)
        return checkAll;

    *resHnd_InOut = newResHnd;
    return checkAll;
};

result
_Pool_Size(
    pool_hnd        poolHnd_In,
    uint64*         size_Out)
{
    if(!l_PoolHead)
        return POOL_NOPOOL;

    pool* target = l_PoolHead;
    while(target->hnd != poolHnd_In && target)
        target = target->pPrev;
    if(!target)
        return POOL_INVREHND;

    *size_Out = (target->oEnd - 1) * POOL_SECTION_SIZE;
    return POOL_SUCCESS;
};

result
_Pool_ResSize(
    res_hnd         resHnd_In,
    uint64*         size_Out)
{
    uint32* res = null;
    if(l_pool_obtainRes(resHnd_In, &res, null, null) != POOL_SUCCESS)
        return POOL_INVREHND;
    *size_Out = (res[RES_SIZE] * POOL_SECTION_SIZE) - RES_META_SIZE;
    return POOL_SUCCESS;
}

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