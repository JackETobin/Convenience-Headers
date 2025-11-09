/*
 * Mem Alloc is built on top of the platform layer, but can be used without it.
 * Note that if you're using this with platform.h, Mem Alloc is designed to be
 * in its own folder [e.g. Mem->mem.alloc.h] such that platform.h is up one
 * level.
 * 
 * Mem Alloc is configured with preprocessor definitions that can be emplaced in
 * whatever file includes Mem Alloc.
 * 
 * Configuration defines that can be used with mem_alloc:
 *  -USE_MEM_TYPES:     includes mem_types.h
 *  -USE_MEM_DEFINES:   includes mem_defines.h
 *  -USE_MEM_DEBUG:     includes mem_debug.h and activates debug functionality.
 *  -USE_PLATFORM:      includes platform.h and uses the OS's virtual alloc
 *                      function as opposed to malloc().
 * 
 * Mem Alloc tracks and tags allocations. All allocations are linked and added
 * to a singly linked list that can be iterated through internally. Allocations
 * are returned as void pointers. Freeing an allocation reduces the size of the
 * allocation to the size of the allocatiion metadata, and the metadata alone is
 * kept in the linked list of allocations that have been made. Freed allocations
 * can be removed from the linked list via a call to _Mem_ClearFree().
 * 
 * Calling _Mem_Kill() frees all allocations, all members of the linked list,
 * and nullifies the linked list.
*/

// TODO: Strip tagging if we aren't in debug mode.
#ifndef MEM_ALLOC_H
#define MEM_ALLOC_H

#if !defined(MEM_HEADERS_H) || !defined(PLATFORM_H)
        #define null            ((void*)0)

    typedef unsigned char       uint8;
    typedef unsigned int        uint32;
    typedef unsigned long long  uint64;

    typedef double              float64;

    typedef uint8               result;
#endif // MEM_TYPES_H || PLATFORM_H
#if !defined(DEFINES_H)
    #define persist         static
    #define internal        static

    #define kb(x)           (x * 1024)
    #define mb(x)           (kb(x) * 1024)
    #define gb(x)           (mb(x) * 1024)

    #define unused(x)           (void)(x)
#endif // DEFINES_H

#define ALLOC_SUCCESS       (result)0X01 // Operation succesful.
#define ALLOC_NOISSUE       (result)0X10 // No overwrite issues detected.
#define ALLOC_OUTOFBOUNDS   (result)0X11 // Memory overwrite detected.
#define ALLOC_NOALLOC       (result)0X12 // Unable to allocate memory.
#define ALLOC_NOFREE        (result)0X12 // Unable to free memory.
#define ALLOC_NOOUTPUT      (result)0X14 // No output buffer provided.
#define ALLOC_NOINPUT       (result)0X15 // Input field missing.
#define ALLOC_UNUSED        (result)0X16 // No allocations made as of yet.
#define ALLOC_NOPREV        (result)0X17 // No previous allocation to iterate to.
#define ALLOC_NONEXT        (result)0X18 // No next allocation to iterate to.

typedef const void**        p_mem;

#if defined(MODE_DEBUG)
    #if !defined(MEM_HEADERS_H)
        #include "../log.h"
        #define DBG(func) _Debug_Catch(func, #func, line, file)
    #endif // MEM_HEADERS_H
#else
    #if !defined(MEM_HEADERS_H)
        #define DBG(func) func
    #endif // MEM_HEADERS_H
#endif // MODE_DEBUG

#if defined(PLATFORM_H) || defined(USE_PLATFORM)
    #include "../platform.h"
    #define Malloc(size_InOut, mem_Out) \
        Plat_Virtual_Alloc(size_InOut, &size_InOut, (p_mem)&mem_Out)
    #define Realloc(size_InOut, mem_In) \
        Plat_Virtual_Realloc(size_InOut, &size_InOut, mem_In)
    #define Free(mem)                   Plat_Virtual_Free(0, (p_mem)&mem)
#else
    #include<stdlib.h>
    internal result
    l_malloc(
        uint64      size_In, 
        void**      mem_Out)
    {
        void* new = malloc(size_In);
        if(new)
        {
            *mem_Out = new;
            return ALLOC_SUCCESS;
        }
        return ALLOC_NOALLOC;
    }
    #define Malloc(size_In, mem_Out)    l_malloc(size_In, (void**)&mem_Out)

    internal result
    l_realloc(
        uint64      size_In, 
        void**      mem_InOut)
    {
        void* new = realloc(size_In, *mem_InOut);
        if(new)
        {
            *mem_InOut = new;
            return ALLOC_SUCCESS;
        }
        return ALLOC_NOALLOC;
    }
    #define Realloc(size_In, mem_In)    l_realloc(size_In, (void**)&mem_In)

    internal result
    l_free(
        void*       mem_In)
    { free(mem_In); return ALLOC_SUCCESS; }
    #define Free(mem)                   l_free((void*)mem)
#endif // PLATFORM_H

#define ALLOC_TAG_SIZE 48
#define DEFAULT_ALIGN 4
#define ALLOC_CHECKVAL 0XBBBBBBBBBBBBBBBB
#define SIZE_META (sizeof(mem_block) + ALLOC_TAG_SIZE)

#define ALLOC_STATUS_READY 0X01
#define ALLOC_STATUS_ISSUE 0X02
#define ALLOC_STATUS_FREED 0X08

typedef struct Mem_Block_St
{
    uint32                  alignment;
    struct Mem_Block_St*    pPrev;
    struct Mem_Block_St*    pNext;
    uint64*                 checkVal;
    uint8                   status;
    char                    tag;
} mem_block;

persist mem_block* l_AllocHead = null;

internal result 
l_mem_copy( 
    const void*     src_In, 
    const void*     dst_In, 
    uint64          range_In)
{
    uint8* src = (uint8*)src_In; uint8* dst = (uint8*)dst_In;
    uint8* lim = (uint8*)(src_In + range_In);
    for(; src < lim; dst++, src++)
        *dst = *src;
    return ALLOC_SUCCESS;
};

internal result 
l_mem_setTag(
    char*           src_In, 
    char*           dst_In)
{
    src_In = (*src_In != '\0') ? (char*)src_In : "Tag Fault";
    char* tagSpcEnd = dst_In + ALLOC_TAG_SIZE - 1;
    for(; *src_In != '\0' && dst_In <= tagSpcEnd; src_In++, dst_In++)
        *dst_In = *src_In;
    *dst_In = '\0';
    return ALLOC_SUCCESS;
};

internal result 
l_mem_alloc(
    uint64*         size_InOut,
    uint32          alignment_In,
    const char*     tag_In, 
    const void**    mem_Out)
{
    uint64 size = *size_InOut + SIZE_META + sizeof(uint64);
    size += alignment_In - (size % alignment_In);
    mem_block* block = null;
    DBG(Malloc(size, block));
    if(!block) 
        return ALLOC_NOALLOC;

    void* mem = (void*)block + SIZE_META;
    size -= (SIZE_META + sizeof(uint64));
    mem_block* prev = l_AllocHead;
    if(prev)
    {
        while(prev->pNext)
            prev = prev->pNext;
        prev->pNext = block;
    } 
    else
        l_AllocHead = block;

    *block = (mem_block){alignment_In, prev, null, mem + size, ALLOC_STATUS_READY, 0};
    *block->checkVal = ALLOC_CHECKVAL;
    l_mem_setTag((char*)tag_In, &block->tag);

    *mem_Out = mem;
    *size_InOut = size;
    return ALLOC_SUCCESS;   
};

// TODO: Hook in the platform realloc.
// NOTE: Platform now extends the virtual alloc, so the realloc methods no longer need a write.
internal result 
l_mem_realloc(
    p_mem           mem_InOut, 
    uint64*         size_InOut,
    uint8           retNull_In)
{
    void* memOld = (void*)*mem_InOut;
    mem_block* blockOld = (memOld - SIZE_META);
    uint64 sizeOld = ((uint8*)blockOld->checkVal - (uint8*)blockOld) + sizeof(uint64);

    uint64 size = *size_InOut + SIZE_META + sizeof(uint64);
    size += blockOld->alignment - (size % blockOld->alignment);
    if(size <= sizeOld)
    {
        *size_InOut = size;
        return ALLOC_SUCCESS;
    }

    mem_block* blockNew = (blockOld == l_AllocHead) ? l_AllocHead : blockOld;
    DBG(Realloc(size, blockNew));
    if(!blockNew) 
        return ALLOC_NOALLOC;
        
    if(blockNew->pPrev) (blockNew->pPrev)->pNext = blockNew;
    if(blockNew->pNext) (blockNew->pNext)->pPrev = blockNew;
        
    void* memNew = ((void*)blockNew) + SIZE_META;
    size -= (SIZE_META + sizeof(uint64));
    blockNew->checkVal = memNew + size;
    *blockNew->checkVal = ALLOC_CHECKVAL;
        
    *mem_InOut = (retNull_In) ? null : memNew;
    *size_InOut = size;
    return ALLOC_SUCCESS;
};

result 
_Mem_Alloc(
    uint64*         size_InOut,
    const char*     tag_In, 
    p_mem           mem_Out)
{
    if(!mem_Out)
        return ALLOC_NOOUTPUT;
    char* tag = (tag_In != null) ? (char*)tag_In : "Untagged";
    return l_mem_alloc(size_InOut, DEFAULT_ALIGN, tag, mem_Out);
};

result 
_Mem_Alloc_Allign(
    uint64*         size_InOut,
    uint32          align_In,
    const char*     tag_In, 
    p_mem           mem_Out)
{
    if(!mem_Out)
        return ALLOC_NOOUTPUT;
    char* tag = (tag_In != null) ? (char*)tag_In : "Untagged";
    return l_mem_alloc(size_InOut, (align_In) ? align_In : DEFAULT_ALIGN, tag, mem_Out);
};

result 
_Mem_Realloc( 
    uint64*         size_InOut,
    p_mem           mem_InOut)
{
    if(!mem_InOut) 
        return ALLOC_NOOUTPUT;
    return l_mem_realloc(mem_InOut, size_InOut, 0);
};

result
_Mem_BlockTag(
    const void*     mem_In, 
    const char*     tag_Out)
{
    if(!mem_In || !tag_Out)
        return (!mem_In) ? ALLOC_NOINPUT : ALLOC_NOOUTPUT;
    mem_block* block = (void*)mem_In - SIZE_META;
    return l_mem_setTag(&block->tag, (char*)tag_Out);
};

result 
_Mem_Zero(
    const void*     mem_In, 
    uint64          range_In)
{
    char* mem = (char*)mem_In;
    char* lim = mem + range_In;
    for(; mem < lim; mem++)
        *mem = 0;
    return ALLOC_SUCCESS;
};

result 
_Mem_CheckBounds(
    void*           mem_In)
{
    mem_block* block = (mem_In - SIZE_META);
    if(*block->checkVal != ALLOC_CHECKVAL)
        block->status = ALLOC_STATUS_ISSUE;
    return (block->status == ALLOC_STATUS_ISSUE) ? ALLOC_OUTOFBOUNDS : ALLOC_SUCCESS;
};

result
_Mem_Next(
    void**          mem_InOut)
{
    if(!mem_InOut || !(void*)*mem_InOut)
        return (!mem_InOut) ? ALLOC_NOOUTPUT : ALLOC_NOINPUT;
    mem_block* block = ((void*)*mem_InOut) - SIZE_META;
    block = block->pNext;
    if(block && block->status & ALLOC_STATUS_FREED)
        block = block->pNext;
    void* mem = (void*)block + SIZE_META;
    *(void**)mem_InOut = (block) ? mem : *mem_InOut;
    return (block) ? ALLOC_SUCCESS : ALLOC_NONEXT;
};

result
_Mem_Prev(
    void**          mem_InOut)
{
    if(!mem_InOut || !(void*)*mem_InOut)
        return (!mem_InOut) ? ALLOC_NOOUTPUT : ALLOC_NOINPUT;
    mem_block* block = ((void*)*mem_InOut) - SIZE_META;
    block = block->pPrev;
    if(block && block->status & ALLOC_STATUS_FREED)
        block = block->pPrev;
    void* mem = (void*)block + SIZE_META;
    *(void**)mem_InOut = (block) ? mem : *mem_InOut;
    return (block) ? ALLOC_SUCCESS : ALLOC_NOPREV;
};

result 
_Mem_ClearFree(void)
{
    for(mem_block* block = l_AllocHead; block; block = block->pNext)
        if(block->status == ALLOC_STATUS_FREED)
        {
            mem_block* bFree = block;
            block = block->pPrev;
            block->pNext = bFree->pNext;
            if(bFree->pNext) 
                (bFree->pNext)->pPrev = block;
            DBG(Free(bFree));
        };
    return ALLOC_SUCCESS;
};

result 
_Mem_Free(
    p_mem           mem_InOut)
{
    if(!mem_InOut || !*mem_InOut) 
        return ALLOC_NOFREE;
    ((mem_block*)((void*)*mem_InOut - SIZE_META))->status = ALLOC_STATUS_FREED;
    l_mem_realloc(mem_InOut, 0, 1);
    return ALLOC_SUCCESS;
};

result
_Mem_Kill(void)
{
    while(l_AllocHead)
    {
        mem_block* target = l_AllocHead;
        l_AllocHead = l_AllocHead->pPrev;
        DBG(Free(target));
    };
    return ALLOC_SUCCESS;
};

uint64 
_Mem_BlockSize(
    const void*     mem_In)
{
    if(!mem_In)
        return 0;
    const mem_block* block = (mem_In - SIZE_META);
    return (void*)block->checkVal - mem_In;
};

#endif // MEM_ALLOC_H