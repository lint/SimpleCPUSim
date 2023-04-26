
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fetch_unit.h"
#include "../cpu.h"
#include "../memory/memory.h"
#include "../misc/misc.h"
#include "../branch_prediction/branch_predictor.h"

// initialize the fetch unit struct
void initFetchUnit(FetchUnit *fetchUnit, int NF) {

    printf_DEBUG(("initializing fetch unit...\n"));

    fetchUnit->NF = NF;
    
    // initialize instruction fetch buffer that is shared between fetch and decode units
    fetchUnit->fetchBuffer = calloc(fetchUnit->NF, sizeof(FetchBufferEntry *));
    fetchUnit->fetchBufferSize = fetchUnit->NF;
    fetchUnit->numInstsInBuffer = calloc(1, sizeof(int));
}

// free any elements of fetch unit that are stored on the heap
void teardownFetchUnit(FetchUnit *fetchUnit) {

    for (int i = 0; i < *fetchUnit->numInstsInBuffer; i++) {
        if (fetchUnit->fetchBuffer[i]) {
            free(fetchUnit->fetchBuffer[i]);
        }
    }

    free(fetchUnit->numInstsInBuffer);
}

// doubles the size of the output buffer if necessary
void extendFetchUnitOutputBufferIfNeeded(FetchUnit *fetchUnit) {

    // only extend the buffer if it is full
    if (*fetchUnit->numInstsInBuffer == fetchUnit->fetchBufferSize) {

        // get old values
        FetchBufferEntry **oldBuffer = fetchUnit->fetchBuffer;
        int oldSize = fetchUnit->fetchBufferSize;
        int newSize = oldSize * 2;

        // reallocate array and copy contents
        fetchUnit->fetchBufferSize = newSize;
        fetchUnit->fetchBuffer = realloc(fetchUnit->fetchBuffer, newSize * sizeof(FetchBufferEntry *));

        for (int i = *fetchUnit->numInstsInBuffer; i < newSize; i++) {
            fetchUnit->fetchBuffer[i] = NULL;
        }

        #ifdef ENABLE_DEBUG_LOG
        printf("extending fetch unit output buffer to %i entries\n", newSize);
        for (int i = 0; i < newSize; i++) {
            printf("buffer entry: %p\n", fetchUnit->fetchBuffer[i]);
        }
        printf("fetchBuffer: %p\n", fetchUnit->fetchBuffer);
        #endif
    }
}

// adds a fetched instruction to the output buffer
void addInstToFetchUnitOutputBuffer(FetchUnit *fetchUnit, char *instStr, int instAddr) {
    extendFetchUnitOutputBufferIfNeeded(fetchUnit);

    FetchBufferEntry *entry = malloc(sizeof(FetchBufferEntry));
    entry->instAddr = instAddr;
    entry->instStr = instStr;

    fetchUnit->fetchBuffer[(*fetchUnit->numInstsInBuffer)++] = entry;

    #ifdef ENABLE_DEBUG_LOG
    printf("added instruction: '%s' addr: '%i' to fetch buffer, numInstsInBuffer: %i\n", instStr, instAddr, *fetchUnit->numInstsInBuffer);
    #endif
}

// remove all entries in the fetch buffer
void flushFetchBuffer(FetchUnit *fetchUnit) {
    
    for (int i = 0; i < *fetchUnit->numInstsInBuffer; i++) {
        fetchUnit->fetchBuffer[i] = NULL;
    }
    *fetchUnit->numInstsInBuffer = 0;
}

// helper method to print the contents of the instruction fetch buffer
void printInstructionFetchBuffer(FetchUnit *fetchUnit) {
    
    printf("instruction fetch buffer: %p, size: %i, numInsts: %i, items: ", fetchUnit->fetchBuffer, fetchUnit->fetchBufferSize, *fetchUnit->numInstsInBuffer);
    
    for (int i = 0; i < *fetchUnit->numInstsInBuffer; i++) {
        printf("%p, ", fetchUnit->fetchBuffer[i]);
    }

    printf("\n");
}

// execute fetch unit's operations during a clock cycle
void cycleFetchUnit(FetchUnit *fetchUnit, RegisterFile *registerFile, InstCache *instCache, BranchPredictor *branchPredictor) {

    printf_DEBUG(("\nperforming fetch unit operations...\n"));

    // get the current value of PC
    int pcVal = readRegisterFileInt(registerFile, PHYS_REG_PC);

    // get the next NF instructions
    for (int i = 0; i < fetchUnit->NF; i++) {

        // get the instruction from the instruction cache
        char *instStr = readInstructionCache(instCache, pcVal);
        if (!instStr) {
            printf_DEBUG(("could not get instruction from cache\n"));
            break;
        }

        // write the instruction to the buffer
        addInstToFetchUnitOutputBuffer(fetchUnit, instStr, pcVal);

        // pcVal += 4;
        pcVal = predictNextPC(branchPredictor, pcVal);
    }
    
    // update the new value of PC in the register file
    writeRegisterFileInt(registerFile, PHYS_REG_PC, pcVal);
 }