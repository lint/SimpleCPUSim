
// #define ENABLE_DEBUG_LOG

#ifdef ENABLE_DEBUG_LOG
#define printf_DEBUG(x)  {  printf("DEBUG:");  printf(x);}
#else
#define printf_DEBUG(x) // nothing
#endif