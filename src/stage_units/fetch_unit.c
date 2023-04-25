
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fetch_unit.h"
#include "../cpu.h"
#include "../memory/memory.h"
#include "../misc/misc.h"
#include "../branch_prediction/branch_predictor.h"

// initialize the fetch unit struct
void initFetchUnit(FetchUnit *fetchUnit, int NF, FetchBufferEntry **fetchBuffer, int *fetchBufferSize, int *numInstsInBuffer) {

    printf("initializing fetch unit...\n");

    fetchUnit->NF = NF;
    fetchUnit->fetchBuffer = fetchBuffer;
    fetchUnit->fetchBufferSize = fetchBufferSize;
    fetchUnit->numInstsInBuffer = numInstsInBuffer;
}

// free any elements of fetch unit that are stored on the heap
void teardownFetchUnit(FetchUnit *fetchUnit) {

}

// doubles the size of the output buffer if necessary
void extendFetchUnitOutputBufferIfNeeded(FetchUnit *fetchUnit) {

    // only extend the buffer if it is full
    if (*fetchUnit->numInstsInBuffer == *fetchUnit->fetchBufferSize) {

        // get old values
        FetchBufferEntry **oldBuffer = fetchUnit->fetchBuffer;
        int oldSize = *fetchUnit->fetchBufferSize;
        int newSize = oldSize * 2;

        // reallocate array and copy contents
        *fetchUnit->fetchBufferSize = newSize;
        fetchUnit->fetchBuffer = realloc(fetchUnit->fetchBuffer, newSize * sizeof(FetchBufferEntry *));

        for (int i = *fetchUnit->numInstsInBuffer; i < newSize; i++) {
            fetchUnit->fetchBuffer[i] = NULL;
        }

        printf("extending fetch unit output buffer to %i entries\n", newSize);
    }
}

// adds a fetched instruction to the output buffer
void addInstToFetchUnitOutputBuffer(FetchUnit *fetchUnit, char *instStr, int instAddr) {
    extendFetchUnitOutputBufferIfNeeded(fetchUnit);

    FetchBufferEntry *entry = malloc(sizeof(FetchBufferEntry));
    entry->instAddr = instAddr;
    entry->instStr = instStr;

    fetchUnit->fetchBuffer[(*fetchUnit->numInstsInBuffer)++] = entry;

    printf("added instruction: '%s' addr: '%i' to fetch buffer, numInstsInBuffer: %i\n", instStr, instAddr, *fetchUnit->numInstsInBuffer);
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