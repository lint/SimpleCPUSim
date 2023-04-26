
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "inst_cache.h"
#include "../misc/misc.h"

// initialize a struct representing the instruction cache
void initInstCache(InstCache *instCache) {
    printf_DEBUG(("initalizing instruction cache...\n"));
    
    instCache->numInsts = 0;
    instCache->cacheSize = INST_CACHE_INITIAL_SIZE;
    instCache->cache = calloc(INST_CACHE_INITIAL_SIZE, sizeof(char *));
}

// free any elements of the instruction cache stored on the heap
void teardownInstCache(InstCache *instCache) {
    if (instCache->cache) {
        free(instCache->cache);
    }
}

// doubles the size of the instruction cache
void extendInstCacheIfNeeded(InstCache *instCache) {

    // only extend the instruction cache if it is currently full
    if (instCache->numInsts >= instCache->cacheSize) {
        
        // get old values
        char **oldCache = instCache->cache;
        int oldSize = instCache->cacheSize;
        int newSize = oldSize * 2;

        // reallocate array and copy contents
        instCache->cacheSize = newSize;
        instCache->cache = calloc(newSize, sizeof(char *));
        memcpy(instCache->cache, oldCache, oldSize * sizeof(char *));
        free(oldCache);

        #ifdef ENABLE_DEBUG_LOG
        printf("extending instruction cache to %i entries\n", newSize);
        #endif
    }
}

// returns an instruction in the instruction cache at the given address
char *readInstructionCache(InstCache *instCache, int address) {
    int index = address / 4; // since there are 4 bytes per instruction being simulated

    if (index < 0 || index >= instCache->numInsts) {
        #ifdef ENABLE_DEBUG_LOG
        printf("error: attempted to read instruction at address '%d' (index '%d') which is out of bounds\n", address, index);
        #endif

        return NULL;
    }

    return instCache->cache[index];
}

// adds a new instruction to the instruction cache when processing the input file
void addInstructionToCache(InstCache *instCache, char *instStr) {

    // extend the instruction cache if not enough space for a new instruction
    extendInstCacheIfNeeded(instCache);

    // inst.addr = instCache->numInsts * 4; // since an instruction is simulated as 4 bytes in memory

    // store the instruction
    instCache->cache[instCache->numInsts++] = instStr;

    #ifdef ENABLE_DEBUG_LOG
    printf("added instruction: '%s' to cache\n", instStr);
    #endif
}