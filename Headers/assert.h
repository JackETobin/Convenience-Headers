#ifndef ASSERT_H
#define ASSERT_H

#if defined(USE_PLATFORM) || defined(USE_ALL)
    #include <stdio.h>
    #define Print(msg, ...) printf(msg, __VA_ARGS__)
#else
    #include <stdio.h>
    #define Print(msg, ...) printf(msg, __VA_ARGS__)
#endif // USE_PLATFORM

#if defined(MEM_ASSERT_WARN)
    #define Assert(exp) ({                                                      \
        if(!exp) {                                                              \
            Print("Assertion warning.\n\t%s%s\n\t%s%s\n\t%s%d\n\n",             \
                "Expression: ", #exp, "File: ", __FILE__, "Line: ", __LINE__);  \
        }                                                                       \
    }) 
#else
    #define Assert(exp) ({                                                      \
        if(!exp) {                                                              \
            pVoid = null;                                                       \
            Print("Assertion failure.\n\t%s%s\n\t%s%s\n\t%s%d\n\n",             \
                "Expression: ", #exp, "File: ", __FILE__, "Line: ", __LINE__);  \
            *pVoid = 1;                                                         \
        }                                                                       \
    })
#endif // MEM_ASSERT_WARN

#endif // MEM_ASSERT_H