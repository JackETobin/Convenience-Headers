#include<stdio.h>

#define USE_MEM_TYPES
#define USE_MEM_DEFINES

#define USE_PLATFORM
#define USE_MEM_DEBUG
#define USE_MEM_ALLOC
#define USE_MEM_POOL

#define DEBUG_LOG_RESULT
#define DEBUG_LOG_VERBOSE
#include "Headers\mem_headers.h"

int 
main(void)
{
#if defined(MEM_ALLOC_H)
    Mem_Debug_Report();
    
    void* a1 = null;
    Mem_Alloc(4, "Alloc 1", (p_mem)&a1);
    uint64 a1Size = Mem_BlockSize(a1);
    printf("%llu\n", a1Size);

    void* a2 = null;
    Mem_Alloc(8, "Alloc 2", (p_mem)&a2);

    void* a3 = null;
    Mem_Alloc(12, "Just a really long tag that should be over the tag limit.", (p_mem)&a3);

    Mem_Realloc(140, (p_mem)&a3);

    void* a4 = null;
    Mem_Alloc_Allign(1600000, 32, "Alloc 4", (p_mem)&a4);
    uint64 a4Size = Mem_BlockSize(a4);
    printf("%llu\n", a4Size);

    Mem_Debug_Report();

    Mem_Realloc(14000, (p_mem)&a2);

    Mem_Free((p_mem)&a3);

    void* a5 = null;
    Mem_Alloc(1200, "Alloc 5", (p_mem)&a5);

    Mem_Debug_Report();

    char tag[MEM_TAG_SIZE];
    Mem_BlockTag(a1, tag);
    printf("%s\n", tag);

    Mem_Next(&a1);
    Mem_BlockTag(a1, tag);
    printf("%s\n", tag);

    Mem_Next(&a1);
    Mem_BlockTag(a1, tag);
    printf("%s\n", tag);

    Mem_Next(&a1);
    Mem_BlockTag(a1, tag);
    printf("%s\n", tag);

    Mem_Prev(&a1);
    Mem_BlockTag(a1, tag);
    printf("%s\n", tag);

    Mem_Prev(&a1);
    Mem_BlockTag(a1, tag);
    printf("%s\n", tag);

    Mem_Prev(&a1);
    Mem_BlockTag(a1, tag);
    printf("%s\n", tag);

    Mem_ClearFree();

    Mem_Debug_Report();

    Mem_Kill();

    Mem_Debug_Report();

#endif // MEM_ALLOC_H

    pool_hnd poolHnd1 = 0;
    Pool_Build(1024, "Pool 1", 0, &poolHnd1);

    pool_hnd poolHnd2 = 0;
    Pool_Build(1024, "Pool 2", 0, &poolHnd2);

    res_hnd resHnd1 = 0;
    Pool_Reserve_At(32, poolHnd1, &resHnd1);

    res_hnd resHnd2 = 0;
    Pool_Reserve(32, &resHnd2);

    res_hnd resHnd3 = 0;
    Pool_Reserve(32, &resHnd3);

    res_hnd resHnd4 = 0;
    Pool_Reserve(32, &resHnd4);

    char* defaultMessage = "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC";
    char* adjustMessage = "AAAAAAAAA";
    char* testRetrieval = null;

    Pool_Write(resHnd4, defaultMessage, 0, 0);
    Pool_Write(resHnd3, defaultMessage, 0, 0);
    Pool_Write(resHnd2, defaultMessage, 0, 0);
    Pool_Write(resHnd1, defaultMessage, 0, 0);

    Pool_Retrieve(resHnd4, (void**)&testRetrieval);
    printf("%s\n", testRetrieval);
    Pool_Retrieve(resHnd3, (void**)&testRetrieval);
    printf("%s\n", testRetrieval);
    Pool_Retrieve(resHnd2, (void**)&testRetrieval);
    printf("%s\n", testRetrieval);
    Pool_Retrieve(resHnd1, (void**)&testRetrieval);
    printf("%s\n\n", testRetrieval);

    Pool_Write(resHnd4, adjustMessage, 9, 6);
    Pool_Write(resHnd3, adjustMessage, 9, 8);
    Pool_Write(resHnd2, adjustMessage, 9, 10);

    char* longMsg = "A longer message that should truncate since it's too long.";
    Pool_Write(resHnd1, longMsg, 59, 0);

    Pool_Retrieve(resHnd4, (void**)&testRetrieval);
    printf("%s\n", testRetrieval);
    Pool_Retrieve(resHnd3, (void**)&testRetrieval);
    printf("%s\n", testRetrieval);
    Pool_Retrieve(resHnd2, (void**)&testRetrieval);
    printf("%s\n", testRetrieval);
    Pool_Retrieve(resHnd1, (void**)&testRetrieval);
    printf("%s\n\n", testRetrieval);

    Pool_Release(resHnd3);
    Pool_Release(resHnd4);

    res_hnd resHnd5 = 0;
    Pool_Reserve(32, &resHnd5);

    Pool_Release(resHnd4);
    Pool_Release(resHnd1);

    Mem_Debug_Report();

    Pool_Kill();

    Mem_Debug_Report();
    
    return 0;
}