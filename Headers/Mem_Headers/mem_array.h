// TODO: Set up errors and input checks.
// TODO: Hook malloc in case this needs to stand up by itself.
#ifndef MEM_ARRAY_H
#define MEM_ARRAY_H

#if !defined(MEM_TYPES_H)
    #define null                ((void*)0)

    typedef unsigned char       uint8;

    typedef int                 int32;
    typedef unsigned int        uint32;

    typedef long long           int64;
    typedef unsigned long long  uint64;

    typedef uint8               result;
#endif
#if !defined(MEM_DEFINES_H)
    #define persist             static
    #define internal            static
    // #define unused(x)           (void)(x)

    #define ARRAY_SUCCESS   (result)0X1C // array success.
    #define ARRAY_NOALLOC   (result)0X1D // Unable to allocate space for the array.
    #define ARRAY_NOFREE    (result)0X1E // An error ocurred while attempting to free the array.
    #define ARRAY_NORESIZE  (result)0X1F // Array was unable to dynamically resize.
    #define ARRAY_EMPTY     (result)0X20 // Array is empty.
#endif
#if defined(USE_MEM_DEBUG) || defined(USE_MEM_ALL)
    #include "mem_debug.h"
    #define DBG(func) _Debug_Catch(func, #func, line, file)
#else
    #define DBG(func) func
#endif // USE_MEM_DEBUG

#define USE_MEM_POOL

typedef struct Array_St {
    uint32      stride;
    uint64      len;
    uint64      max;
#if defined(USE_MEM_POOL) || defined(USE_MEM_ALL)
    res_hnd     res;
#endif
} array;

typedef void*                   array_hnd;

#if defined(USE_MEM_POOL) || defined(USE_MEM_ALL)
    #include "mem_pool.h"

    #define DES_POOL_NONE       2
    #define DES_POOL_DEFAULT    3
    #define DES_POOL_MANUAL     4

    typedef struct Designated_Pool_St {
        pool_hnd    def;
        pool_hnd    des;
        uint32      count;
        uint8       set;
    } des_pool;

    persist des_pool l_DesPool = { 0, 0, 0, DES_POOL_NONE };
    
    internal result
    l_array_alloc(
        uint64      size_In,
        array**      array_Out)
    {
        res_hnd res = 0;
        size_In += sizeof(array);
        if(l_DesPool.set == DES_POOL_NONE)
        {
            if(DBG(_Pool_Build(
                size_In, 
                "Array default", 
                POOL_DYNAMIC, 
                &l_DesPool.def) != POOL_SUCCESS))
                return ARRAY_NOALLOC;
            l_DesPool.set = DES_POOL_DEFAULT;
        }
        array* target = null;
        pool_hnd pool = (l_DesPool.set == DES_POOL_MANUAL) ? 
            l_DesPool.des : l_DesPool.def;
        if(DBG(_Pool_Reserve_At(size_In, pool, &res) != POOL_SUCCESS))
            return ARRAY_NOALLOC;
        if(DBG(_Pool_Retrieve(res, (void**)&target) != POOL_SUCCESS))
            return ARRAY_NOALLOC;
        target->res = res;
        *array_Out = target;
        return ARRAY_SUCCESS;
    };

    internal result
    l_array_realloc(
        uint64 size_In, 
        array** array_InOut)
    {
        size_In += sizeof(array);
        array* target = *array_InOut;
        if(DBG(_Pool_Modify(&target->res, size_In)) != POOL_SUCCESS)
            return ARRAY_NORESIZE;
        if(DBG(_Pool_Retrieve(target->res, (void**)array_InOut) != POOL_SUCCESS))
            return ARRAY_NOALLOC;
        return ARRAY_SUCCESS;
    }

    internal result
    l_array_free(
        array_hnd*  data_In)
    {
        array* data = (void*)(*data_In) - sizeof(array);
        if(DBG(_Pool_Release(&data->res)) != POOL_SUCCESS)
            return ARRAY_NOFREE;
        *data_In = null;
        return ARRAY_SUCCESS;
    }

    result
    _Array_DesignatePool(
        pool_hnd    poolHnd_In)
    {
        l_DesPool = (des_pool){ l_DesPool.def, poolHnd_In, l_DesPool.count, DES_POOL_MANUAL };
        return ARRAY_SUCCESS;
    };
#else
    #include "stdlib.h"
    #define Array_Alloc()       malloc()
    #define Array_Free()        free()
    #define Array_Realloc()     realloc()
#endif

result
_Array_Build(
    uint32          stride_In, 
    uint32          count_In, 
    array_hnd*      hnd_Out)
{
    array* new = null;
    DBG(l_array_alloc(stride_In * count_In, &new));
    new->len = 0;
    new->max = count_In;
    new->stride = stride_In;

    *hnd_Out = (void*)new + sizeof(array);
    return ARRAY_SUCCESS;
};

result
_Array_Destroy(
    array_hnd*      hnd_InOut)
{ return l_array_free(hnd_InOut); };

result
_Array_Push(
    array_hnd*      hnd_InOut, 
    void*           data_In)
{
    array* tar = *hnd_InOut - sizeof(array);
    if(tar->len >= tar->max)
    {
        l_array_realloc(tar->max * (tar->stride * 1.5), &tar);
        *hnd_InOut = (uint8*)tar + sizeof(array);
    }
    array_hnd hnd = *hnd_InOut;
    for(uint8* i = hnd + (tar->len * tar->stride); i >= (uint8*)hnd; i--)
        *(i + tar->stride) = *i;
    for(uint32 i = 0; i < tar->stride; i++)
        *((uint8*)hnd + i) = *((uint8*)data_In + i);
    
    tar->len++;
    return ARRAY_SUCCESS;
};

result
_Array_PushBack(
    array_hnd*      hnd_InOut,
    void*           data_In)
{
    array* tar = *hnd_InOut - sizeof(array);
    if(tar->len >= tar->max)
    {
        l_array_realloc(tar->max * (tar->stride * 1.5), &tar);
        *hnd_InOut = (uint8*)tar + sizeof(array);
    }
    uint8* dst = *(uint8**)hnd_InOut + (tar->stride * tar->len);
    for(uint32 i = 0; i < tar->stride; i++)
        *(dst + i) = *((uint8*)data_In + i);
    
    tar->len++;
    return ARRAY_SUCCESS;
};

result
_Array_Pop(
    array_hnd       hnd_In, 
    void*           data_Out)
{
    array* tar = hnd_In - sizeof(array);
    uint8* hnd = (uint8*)hnd_In;
    for(uint64 i = 0; i < tar->stride; i++)
        *((uint8*)data_Out + i) = *(hnd + i);

    uint8* lim = hnd + (tar->len * tar->stride);
    for(; hnd < lim; hnd++)
        *hnd = *(hnd + tar->stride);

    uint8* back = lim - tar->stride;
    for(; back < lim; back++)
        *back = 0;
    
    tar->len--;
    return ARRAY_SUCCESS;
}

result
_Array_PopBack(
    array_hnd       hnd_In, 
    void*           data_Out)
{
    array* tar = hnd_In - sizeof(array);
    uint64 lim = tar->len * tar->stride;
    uint64 back = lim - tar->stride;
    for(; back < lim; back++)
    {
        *((uint8*)data_Out + back) = *((uint8*)hnd_In + back);
        *((uint8*)hnd_In + back) = 0;
    }
    tar->len--;
    return ARRAY_SUCCESS;
}

result
_Array_Insert(
    array_hnd*      hnd_InOut,
    uint32          element_In,
    void*           data_In)
{
    array* tar = *hnd_InOut - sizeof(array);
    if(tar->len >= tar->max)
    {
        l_array_realloc(tar->max * (tar->stride * 1.5), &tar);
        *hnd_InOut = (uint8*)tar + sizeof(array);
    }
    uint8* hnd = (uint8*)*hnd_InOut;
    uint8* lim = hnd + (element_In * tar->stride);
    for(uint8* i = hnd + (tar->len * tar->stride); i >= lim; i--)
        *(i + tar->stride) = *i;
    for(uint32 i = 0; i < tar->stride; i++)
        *(lim + i) = *((uint8*)data_In + i);
    
    tar->len++;
    return ARRAY_SUCCESS;
}

result
_Array_Remove(
    array_hnd       hnd_In,
    uint32          element_In)
{
    array* tar = hnd_In - sizeof(array);
    uint8* lim = (uint8*)hnd_In + (tar->len * tar->stride);
    for(uint8* i = (uint8*)hnd_In + (element_In * tar->stride); i < lim; i++)
        *i = *(i + tar->stride);

    uint8* back = lim - tar->stride;
    for(; back < lim; back++)
        *back = 0;
    
    tar->len--;
    return ARRAY_SUCCESS;
}

result
_Array_Length(
    array_hnd       hnd_In,
    uint32*         len_Out)
{
    array* tar = hnd_In - sizeof(array);
    *len_Out = tar->len;
    return ARRAY_SUCCESS;
}

result
_Array_Size(
    array_hnd       hnd_In,
    uint64*         size_Out)
{
    array* tar = hnd_In - sizeof(array);
    *size_Out = tar->len * tar->stride;
    return ARRAY_SUCCESS;
}

result
_Array_Stride(
    array_hnd       hnd_In,
    uint32*         stride_Out)
{
    array* tar = hnd_In - sizeof(array);
    *stride_Out = tar->stride;
    return ARRAY_SUCCESS;
}

#endif // MEM_ARRAY_H