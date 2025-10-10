#ifndef MEM_DEBUG_H
#define MEM_DEBUG_H

#include "mem_defines.h"
#include "mem_types.h"

const char* l_ErrMsg[] = {
    // MEM_ALLOC_H
    "Operation succesful.",
    "No overwrite issues detected.",
    "Memory overwrite detected.",
    "Unable to allocate memory.",
    "Unable to free memory.",
    "No output buffer provided.",
    "Input field missing.",
    "No allocations made as of yet.",
    "No previous allocation to iterate to.",
    "No next allocation to iterate to.",

    // MEM_POOL_H
    "Memory pool success.",
    "No space available.",
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
    "Memory pool critical failure."
};

#if defined(PLATFORM_H) || defined(MEM_DEBUG)
    #include <stdio.h>
    #define Print(s, ...) printf(s, __VA_ARGS__);
#else
    #include <stdio.h>
    #define Print(s, ...) printf(s, __VA_ARGS__);
#endif // PLATFORM_H || MEM_DEBUG

result
_Debug_Catch(result res_In, char* call_In, int32 line_In, char* file_In)
{
#if !defined(DEBUG_LOG_VERBOSE)
    if(res_In == POOL_SUCCESS || res_In == MEM_SUCCESS)
        return res_In;
#endif // DEBUG_LOG_VERBOSE

    Print("File: %s\nLine: %d\nCall: %s\nResult: %s\n\n", file_In, line_In, call_In, l_ErrMsg[res_In]);
    return res_In;
};

internal void 
l_mem_sizeStr(
    uint64          size_In,
    float64*        size_Out,
    char*           unit_Out)
{
    char* unit = null;
    float64 size = size_In;
    if((size_In / gb(1)) > 0)
    { unit = "GB"; *size_Out = size / gb(1); }
    if((size_In / mb(1)) > 0 && !unit)
    { unit = "MB"; *size_Out = size / mb(1); }
    if((size_In / kb(1)) > 0 && !unit)
    { unit = "KB"; *size_Out = size / kb(1); }
    if(!unit) 
    { unit = " B"; *size_Out = size; }

    for(uint32 i = 0; i < 3; i++)
        unit_Out[i] = unit[i];
    return;
};

#if defined(USE_MEM_ALLOC) || defined(USE_MEM_ALL)
#include "mem_alloc.h"

internal void 
l_mem_statusStr(
    uint8           status_In,
    char*           status_Out) // NOTE: Buffer size for status out must be greater than 26.
{
    char* status = null;
    status = (status_In & MEM_STATUS_READY) ? "READY" : status;
    status = (status_In & MEM_STATUS_ISSUE) ? "ISSUE" : status;
    status = (status_In & MEM_STATUS_FREED) ? "FREE" : status;
    for(; *status != '\0'; status_Out++, status++)
        *status_Out = *status;
    *status_Out = '\0';
    return;
};

internal void
l_mem_alloc_printInfo(void)
{
    if(l_AllocHead)
    {
        char unit[3];
        float64 size = 0; 
        uint64 netSize = 0; 
        uint32 index = 0, count = 0;
        Print("\n%-72s%-13s%-7s%-7s%s\n%-72s%-13s%-7s%-7s%s\n", 
            "TAG", "SIZE", "ALIGN", "INDEX", "STATUS", "---", "----", "-----", "-----", "------");
        for(mem_block* block = l_AllocHead; block; block = block->pNext, index++)
        {
            char unit[3];
            char status[26];
            uint64 sizeBytes = (void*)block->checkVal - ((void*)block + SIZE_META);
            l_mem_sizeStr(sizeBytes, &size, unit);
            _Mem_CheckBounds(((void*)block) + SIZE_META);
            l_mem_statusStr(block->status, status);
            Print("%-72s%-9.3f%-4s%-7u%-6u%s\n", 
                &block->tag, size, unit, block->alignment, index, status);
            netSize += sizeBytes; count += (block->status != MEM_STATUS_FREED);
        }
        l_mem_sizeStr(netSize, &size, unit);
        Print("\n%-72s%-9.3f%-3s\n%-85s%u\n\n", 
            "Allocated total:", size, unit, "Allocation Count:", count);
        return;
    }
    Print("\nNo allocations present.\n");
    return;
};
#endif // USE_MEM_ALLOC

#if defined(USE_MEM_POOL) || defined(USE_MEM_ALL)
#include "mem_pool.h"

internal void
l_mem_pool_printInfo(void)
{
    pool* target = l_PoolHead;
    if(target)
    {
        while(target)
        {
            #if defined(USE_MEM_ALLOC) || defined(USE_MEM_ALL)
                char tag[MEM_TAG_SIZE]; 
                _Mem_BlockTag(target, tag);
            #else
                char* tag = "Untagged";
            #endif // USE_MEM_ALLOC
            uint64 spaceTotal = (target->oEnd - 1) * POOL_SECTION_SIZE;
            uint64 spaceRemaining = (target->oEnd - target->oData) * POOL_SECTION_SIZE;
            uint32 resCount = target->resCount;
            uint32 voidCount = target->voids[1];
            Print("%s%u\n%s%s\n%s%llu\n%s%llu\n%s%u\n%s%u\n\n",
                "Pool: ", target->hnd,
                "Tag: ", tag,
                "Size Total: ", spaceTotal,
                "Size Remaining: ", spaceRemaining,
                "Reservation Count: ", target->resCount,
                "VoidCount: ", target->voids[1]);
            target = target->pPrev;
        }
        return;
    }
    Print("\nNo memory pools present.\n");
    return;
};

#endif // USE_MEM_POOL

void
_Debug_Report(void)
{
Print("\n///////////////DEBUG REPORT///////////////\n\n");
#if defined(PLATFORM_H) || defined(USE_PLATFORM) || defined(USE_MEM_ALL)

#endif // USE_PLATFORM
#if defined(USE_MEM_ALLOC) || defined(USE_MEM_ALL)
    Print("\n/////////////MEM ALLOC BEGIN//////////////\n");
    l_mem_alloc_printInfo();
    Print("\n//////////////MEM ALLOC END///////////////\n\n");
#endif // USE_MEM_ALLOC
#if defined(USE_MEM_POOL) || defined(USE_MEM_ALL)
    Print("\n//////////////MEM POOL BEGIN//////////////\n");
    l_mem_pool_printInfo();
    Print("\n///////////////MEM POOL END///////////////\n\n");
#endif // USE_MEM_ALLOC
};

#endif // MEM_DEBUG_H