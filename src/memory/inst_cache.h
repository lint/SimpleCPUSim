
#define INST_CACHE_INITIAL_SIZE 256 // initial number of instructions to store in the instruction cache

typedef struct Instruction Instruction; // forward declaration

// struct representing an instruction cache
typedef struct InstCache {
    char **cache;
    int cacheSize;
    int numInsts;
} InstCache;

// instruction cache methods
void initInstCache(InstCache *instCache);
void teardownInstCache(InstCache *instCache);
void extendInstCacheIfNeeded(InstCache *instCache);
char *readInstructionCache(InstCache *instCache, int address);
void addInstructionToCache(InstCache *instCache, char *instStr);