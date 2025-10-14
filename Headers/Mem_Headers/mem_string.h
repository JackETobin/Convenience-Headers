#ifndef MEM_STRING_H
#define MEM_STRING_H

#if !defined(MEM_TYPES_H)
    typedef unsigned char       uint8;

    typedef unsigned int        int32;
    typedef unsigned int        uint32;

    typedef unsigned long long  int64;
    typedef unsigned long long  uint64;

    typedef float               float32;
    typedef double              float64;

    typedef uint8               result;
#endif // MEM_TYPES_H

#define STRING_INT32_BUFFER 12
#define STRING_INT64_BUFFER 22

#define STRING_SUCCESS      (result)0X01
#define STRING_TRUNCATE     (result)0X02

// Buffer size int32: 12
// Buffer size int64: 22
/*
 * If you are giving this function a 32-bit integer, the str_Out buffer size
 * should be STRING_INT32_BUFFER. For a 64-bit integer the str_Out buffer size
 * should be STRING_INT64_BUFFER.
 */
result
_String_Itoa(int64 int64_In, uint32 buffSize_In, char* str_Out)
{
    uint64 divisor = 1;
    uint32 strBack = 0;
    if(int64_In < 0)
    {
        str_Out[0] = '-';
        int64_In *= -1;
        strBack++;
    }
    uint64 toStr = int64_In;
    while(int64_In > 10)
    {
        divisor *= 10;
        int64_In /= 10;
    }
    char* numTable = "0123456789";
    for(; strBack < buffSize_In && divisor; strBack++)
    {
        uint64 digit = (toStr / divisor) % 10;
        str_Out[strBack] = numTable[digit];
        divisor /= 10;
    }
    str_Out[strBack] = '\0';
    return (strBack < buffSize_In) ? STRING_SUCCESS : STRING_TRUNCATE;
};

result
_String_Ftoa();

result
_String_Dtoa();

result
_String_Atoi();

result
_String_Atof();

result
_String_Atod();

result
_String_Comp();

result
_String_TrimLeading();

result
_String_TrimTrailing();

result
_String_Trim();

result
_String_RemoveSpaces();

result
_String_Concat(char* strArray_In[], uint32 count_In, uint64 buffSize_In, char* str_Out)
{
    char* dst = str_Out;
    uint64 strBack = 0;
    for(uint32 i = 0; i < count_In; i++)
    {
        char* src = strArray_In[i];
        while(*src != '\0' && strBack < buffSize_In)
        {
            *dst = *src;
            dst++; src++;
            strBack++;
        }
    }
    str_Out[strBack] = '\0';
    return (strBack < buffSize_In) ? STRING_SUCCESS : STRING_TRUNCATE;
};

#endif // MEM_STRING_H