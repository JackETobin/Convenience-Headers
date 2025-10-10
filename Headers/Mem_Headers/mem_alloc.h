// TODO: Write out the documentation.
// TODO: Add a TAG TRUNCATION warning to the results, and to the tagging mechanism.
/*
// NOTE: If you're using this along with other custom headers, put them in the same folder.


*/

#ifndef MEM_ALLOC_H
#define MEM_ALLOC_H

#if defined(USE_MEM_TYPES) || defined(USE_MEM_ALL)
    #include "mem_types.h"
#else
    #define null                ((void*)0)

    typedef unsigned char       uint8;
    typedef unsigned int        uint32;
    typedef unsigned long long  uint64;

    typedef double              float64;

    typedef uint8               result;
#endif // USE_MEM_TYPES

typedef const void**        p_mem;

#if defined(USE_MEM_DEFINES) || defined(USE_MEM_ALL)
    #include "mem_defines.h"
#else
    #define persist         static
    #define internal        static

    #define kb(x)           (x * 1024)
    #define mb(x)           (kb(x) * 1024)
    #define gb(x)           (mb(x) * 1024)
    
    #define unused(x)           (void)(x)

    #define MEM_SUCCESS     (result)0X00 // Operation succesful.
    #define MEM_NOISSUE     (result)0X01 // No overwrite issues detected.
    #define MEM_OUTOFBOUNDS (result)0X02 // Memory overwrite detected.
    #define MEM_NOALLOC     (result)0X03 // Unable to allocate memory.
    #define MEM_NOFREE      (result)0X04 // Unable to free memory.
    #define MEM_NOOUTPUT    (result)0X05 // No output buffer provided.
    #define MEM_NOINPUT     (result)0X06 // Input field missing.
    #define MEM_UNUSED      (result)0X07 // No allocations made as of yet.
    #define MEM_NOPREV      (result)0X08 // No previous allocation to iterate to.
    #define MEM_NONEXT      (result)0X09 // No next allocation to iterate to.
#endif // USE_MEM_DEFINES

#if defined(PLATFORM_H) || defined(USE_PLATFORM)
    #include "../platform.h"
    #define Malloc(size_InOut, mem_Out) Plat_Virtual_Alloc(size_InOut, null, &size_InOut, (const void**)&mem_Out)
    #define Free(mem)                   Plat_Virtual_Free(0, mem)
#else
    #include<stdlib.h>
    #define Malloc(size_In, mem_Out)    mem_Out = malloc(size_In)
    #define Free(mem)                   free(mem)
#endif

#if defined(USE_MEM_DEBUG) || defined(USE_MEM_ALL)
    #include "mem_debug.h"
#else
    #define DBG(func) func
#endif

#define MEM_TAG_SIZE 48
#define DEFAULT_ALIGN 4
#define MEM_CHECKVAL 0XBBBBBBBBBBBBBBBB
#define SIZE_META (sizeof(mem_block) + MEM_TAG_SIZE)

#define MEM_STATUS_READY 0X01
#define MEM_STATUS_ISSUE 0X02
// #define STATUS_ZEROD 0X04
#define MEM_STATUS_FREED 0X08

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

result 
_Mem_Alloc(
    uint64          size_In, 
    const char*     tag_In, 
    p_mem           mem_Out);

result 
_Mem_Alloc_Allign(
    uint64          size_In,
    uint32          align_In,
    const char*     tag_In,
    p_mem           mem_Out);

result 
_Mem_Realloc( 
    uint64          size_In,
    p_mem           mem_InOut);

result 
_Mem_Free(
    p_mem           mem_InOut);

result                      // Takes a pointer and a buffer for the tag string, the buffer must be at least 48 bytes [MEM_TAG_SIZE] long.
_Mem_BlockTag(
    const void*     mem_In, 
    const char*     tag_Out);

result 
_Mem_Zero(
    const void*     mem_In, 
    uint64          range_In);

result 
_Mem_CheckBounds(
    void*           mem_In);

result
_Mem_Next(
    void**          mem_InOut);

result
_Mem_Prev(
    void**          mem_InOut);

result 
_Mem_ClearFree(void);

result
_Mem_Kill(void);

uint64 
_Mem_BlockSize(
    const void*     mem_In);

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
    return MEM_SUCCESS;
};

internal result 
l_mem_setTag(
    char*           src_In, 
    char*           dst_In)
{
    src_In = (*src_In != '\0') ? (char*)src_In : "Tag Fault";
    char* tagSpcEnd = dst_In + MEM_TAG_SIZE - 1;
    for(; *src_In != '\0' && dst_In <= tagSpcEnd; src_In++, dst_In++)
        *dst_In = *src_In;
    *dst_In = '\0';
    return MEM_SUCCESS;
};

internal result 
l_mem_alloc(
    uint64          size_In,
    uint32          alignment_In,
    const char*     tag_In, 
    const void**    mem_Out)
{
    size_In += alignment_In - (size_In % alignment_In);
    size_In += SIZE_META + sizeof(uint64);
    mem_block* block = null;
    Malloc(size_In, block);
    if(!block) 
        return MEM_NOALLOC;

    void* mem = (void*)block + SIZE_META;
    size_In -= (SIZE_META + sizeof(uint64));
    mem_block* prev = l_AllocHead;
    if(prev)
    {
        while(prev->pNext)
            prev = prev->pNext;
        prev->pNext = block;
    } 
    else
        l_AllocHead = block;

    *block = (mem_block){alignment_In, prev, null, mem + size_In, MEM_STATUS_READY, 0};
    *block->checkVal = MEM_CHECKVAL;
    l_mem_setTag((char*)tag_In, &block->tag);

    *mem_Out = mem;
    return MEM_SUCCESS;   
};

internal result 
l_mem_realloc(
    p_mem           mem_InOut, 
    uint64          size_In,
    uint8           retNull_In)
{
    void* memOld = (void*)*mem_InOut;
    mem_block* blockOld = (memOld - SIZE_META);
    uint64 sizeOld = ((uint8*)blockOld->checkVal - (uint8*)blockOld) + sizeof(uint64);

    size_In += blockOld->alignment - (size_In % blockOld->alignment);
    size_In += SIZE_META + sizeof(uint64);
    if(size_In <= sizeOld)
        return MEM_SUCCESS;

    mem_block* blockNew = null; 
    Malloc(size_In, blockNew);
    if(!blockNew) 
        return MEM_NOALLOC;

    l_mem_copy(blockOld, blockNew, ((size_In < sizeOld) ? size_In : sizeOld));
    Free(blockOld);
        
    if(blockNew->pPrev) (blockNew->pPrev)->pNext = blockNew;
    if(blockNew->pNext) (blockNew->pNext)->pPrev = blockNew;
        
    void* memNew = ((void*)blockNew) + SIZE_META;
    size_In -= (SIZE_META + sizeof(uint64));
    blockNew->checkVal = memNew + size_In;
    *blockNew->checkVal = MEM_CHECKVAL;
        
    *mem_InOut = (retNull_In) ? null : memNew;
    return MEM_SUCCESS;
};

result 
_Mem_Alloc(
    uint64          size_In,
    const char*     tag_In, 
    p_mem           mem_Out)
{
    if(!mem_Out)
        return MEM_NOOUTPUT;
    char* tag = (tag_In != null) ? (char*)tag_In : "Untagged";
    return l_mem_alloc(size_In, DEFAULT_ALIGN, tag, mem_Out);
};

result 
_Mem_Alloc_Allign(
    uint64          size_In,
    uint32          align_In,
    const char*     tag_In, 
    p_mem           mem_Out)
{
    if(!mem_Out)
        return MEM_NOOUTPUT;
    char* tag = (tag_In != null) ? (char*)tag_In : "Untagged";
    return l_mem_alloc(size_In, (align_In) ? align_In : DEFAULT_ALIGN, tag, mem_Out);
};

result 
_Mem_Realloc( 
    uint64          size_In,
    p_mem           mem_InOut)
{
    if(!mem_InOut) 
        return MEM_NOOUTPUT;
    return l_mem_realloc(mem_InOut, size_In, 0);
};

result
_Mem_BlockTag(
    const void*     mem_In, 
    const char*     tag_Out)
{
    if(!mem_In || !tag_Out)
        return (!mem_In) ? MEM_NOINPUT : MEM_NOOUTPUT;
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
    return MEM_SUCCESS;
};

result 
_Mem_CheckBounds(
    void*           mem_In)
{
    mem_block* block = (mem_In - SIZE_META);
    if(*block->checkVal != MEM_CHECKVAL)
        block->status = MEM_STATUS_ISSUE;
    return (block->status == MEM_STATUS_ISSUE) ? MEM_OUTOFBOUNDS : MEM_SUCCESS;
};

result
_Mem_Next(
    void**          mem_InOut)
{
    if(!mem_InOut || !(void*)*mem_InOut)
        return (!mem_InOut) ? MEM_NOOUTPUT : MEM_NOINPUT;
    mem_block* block = ((void*)*mem_InOut) - SIZE_META;
    block = block->pNext;
    if(block && block->status & MEM_STATUS_FREED)
        block = block->pNext;
    void* mem = (void*)block + SIZE_META;
    *(void**)mem_InOut = (block) ? mem : *mem_InOut;
    return (block) ? MEM_SUCCESS : MEM_NONEXT;
};

result
_Mem_Prev(
    void**          mem_InOut)
{
    if(!mem_InOut || !(void*)*mem_InOut)
        return (!mem_InOut) ? MEM_NOOUTPUT : MEM_NOINPUT;
    mem_block* block = ((void*)*mem_InOut) - SIZE_META;
    block = block->pPrev;
    if(block && block->status & MEM_STATUS_FREED)
        block = block->pPrev;
    void* mem = (void*)block + SIZE_META;
    *(void**)mem_InOut = (block) ? mem : *mem_InOut;
    return (block) ? MEM_SUCCESS : MEM_NOPREV;
};

result 
_Mem_ClearFree(void)
{
    for(mem_block* block = l_AllocHead; block; block = block->pNext)
        if(block->status == MEM_STATUS_FREED)
        {
            mem_block* bFree = block;
            block = block->pPrev;
            block->pNext = bFree->pNext;
            if(bFree->pNext) 
                (bFree->pNext)->pPrev = block;
            Free((void*)bFree);
        };
    return MEM_SUCCESS;
};

result 
_Mem_Free(
    p_mem           mem_InOut)
{
    if(!mem_InOut || !*mem_InOut) 
        return MEM_NOFREE;
    ((mem_block*)((void*)*mem_InOut - SIZE_META))->status = MEM_STATUS_FREED;
    l_mem_realloc(mem_InOut, 0, 1);
    return MEM_SUCCESS;
};

result
_Mem_Kill(void)
{
    while(l_AllocHead)
    {
        mem_block* target = l_AllocHead;
        l_AllocHead = l_AllocHead->pPrev;
        Free(target);
    };
    return MEM_SUCCESS;
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