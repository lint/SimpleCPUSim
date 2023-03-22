
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "inst_cache.h"
#include "../types/types.h"

// initialize a struct representing the instruction cache
void initInstCache(InstCache *instCache) {
    printf("initalizing instruction cache...\n");
    instCache->numInsts = 0;
    instCache->cacheSize = INST_CACHE_INITIAL_SIZE;
    instCache->cache = calloc(INST_CACHE_INITIAL_SIZE, sizeof(Instruction));
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
        Instruction *oldCache = instCache->cache;
        int oldSize = instCache->cacheSize;
        int newSize = oldSize * 2;

        // reallocate array and copy contents
        instCache->cacheSize = newSize;
        instCache->cache = calloc(newSize, sizeof(Instruction));
        memcpy(instCache->cache, oldCache, oldSize * sizeof(Instruction));
        free(oldCache);

        printf("extending instruction cache to %i entries\n", newSize);
    }
}

// returns an instruction in the instruction cache at the given address
Instruction *readInstructionCache(InstCache *instCache, int address) {
    int index = address / 4; // since there are 4 bytes per instruction being simulated

    if (index < 0 || index >= instCache->numInsts) {
        printf("error: attempted to read instruction at address '%d' (index '%d') which is out of bounds\n", address, index);
        return NULL;
    }

    return &(instCache->cache[index]);
}

// adds a new instruction to the instruction cache when processing the input file
void addInstructionToCache(InstCache *instCache, Instruction inst) {

    // extend the instruction cache if not enough space for a new instruction
    extendInstCacheIfNeeded(instCache);

    // store the instruction
    instCache->cache[instCache->numInsts++] = inst;
}

// give each branch instruction the target address based on the label
int resolveInstLabels(InstCache *instCache) {

    // iterate through every instruction
    for (int i = 0; i < instCache->numInsts; i++) {
        Instruction *inst1 = &instCache->cache[i];
        
        // check if the instruction is a branch
        if (inst1->type == BNE) {

            int foundMatch = 0;

            // check every other instruction
            for (int j = 0; j < instCache->numInsts; j++) {
                if (j == i) {
                    continue;
                }

                Instruction *inst2 = &instCache->cache[j];

                // check if labels match
                if (!strcmp(inst1->branchTargetLabel, inst2->label)) {
                    // printf("instruction: %i (address: %i) has the target label: %s matched by branch: %i\n", j, j * 4, inst2->label, i);

                    inst1->branchTargetAddr = j * 4; // simulating instructions taking 4 bytes
                    foundMatch = 1;
                    break;
                }
            }

            // return error if there is instruction that has the target label
            if (!foundMatch) {
                printf("error: branch could not find instruction with label: %s\n", inst1->branchTargetLabel);
                return 1;
            }
        }
    }

    return 0;
}