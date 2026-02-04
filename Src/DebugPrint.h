#ifndef DEBUG_PRINT 
#define DEBUG_PRINT

#define DEBUG 1

#ifdef DEBUG
#define debugPrintf(...) printf("DEBUG: " __VA_ARGS__)
#define debugPrint(x) printf("%s\n", x)
#else
#define debugPrintf(...) \
    do                   \
    {                    \
    } while (0)
#define debugPrint(x) \
    do                \
    {                 \
    } while (0)
#endif

#endif