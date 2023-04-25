
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

    printf("initializing fetch unit...\n");

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

        printf("extending fetch unit output buffer to %i entries\n", newSize);
        
        for (int i = 0; i < newSize; i++) {
            printf("buffer entry: %p\n", fetchUnit->fetchBuffer[i]);
        }

        printf("fetchBuffer: %p\n", fetchUnit->fetchBuffer);
    }
}

// adds a fetched instruction to the output buffer
void addInstToFetchUnitOutputBuffer(FetchUnit *fetchUnit, char *instStr, int instAddr) {
    extendFetchUnitOutputBufferIfNeeded(fetchUnit);

        printf("here\n");


    FetchBufferEntry *entry = malloc(sizeof(FetchBufferEntry));
    entry->instAddr = instAddr;
    entry->instStr = instStr;

        printf("here2\n");

    printf("numInstsInBuffer: %i\n", *fetchUnit->numInstsInBuffer);
    printf("fetchBufferSize: %i\n", fetchUnit->fetchBufferSize);

    printf("entry: %p\n", entry);

    for (int i = 0; i < fetchUnit->fetchBufferSize; i++) {
        printf("curr entries: %p", fetchUnit->fetchBuffer[i]);
    }


    fetchUnit->fetchBuffer[(*fetchUnit->numInstsInBuffer)++] = entry;

        printf("here3\n");


    printf("added instruction: '%s' addr: '%i' to fetch buffer, numInstsInBuffer: %i\n", instStr, instAddr, *fetchUnit->numInstsInBuffer);
}

// remove all entries in the fetch buffer
void flushFetchBuffer(FetchUnit *fetchUnit) {
    // clear fetch buffer
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

    printf("\nperforming fetch unit operations...\n");

    // get the current value of PC
    int pcVal = readRegisterFileInt(registerFile, PHYS_REG_PC);

    // get the next NF instructions
    for (int i = 0; i < fetchUnit->NF; i++) {

        // get the instruction from the instruction cache
        char *instStr = readInstructionCache(instCache, pcVal);
        if (!instStr) {
            printf("could not get instruction from cache\n");
            break;
        }

        // write the instruction to the buffer
        addInstToFetchUnitOutputBuffer(fetchUnit, instStr, pcVal);

        // pcVal += 4;
        pcVal = predictNextPC(branchPredictor, pcVal);
    }
    
    // update the new value of PC in the register file
    writeRegisterFileInt(registerFile, PHYS_REG_PC, pcVal);

    // printf("instructions in fetch unit output buffer: \n");
    // for (int i = 0; i < fetchUnit->numInstsInBuffer; i++) {
    //     printInstruction(*(fetchUnit->outputBuffer[i]));
    // }
 }