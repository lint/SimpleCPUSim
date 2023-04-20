
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fetch_unit.h"
#include "../cpu.h"
#include "../memory/memory.h"
#include "../misc/misc.h"

// initialize the fetch unit struct
void initFetchUnit(FetchUnit *fetchUnit, int NF, Instruction **instFetchBuffer, int *instFetchBufferSize, int *numInstsInBuffer) {

    printf("initializing fetch unit...\n");

    fetchUnit->NF = NF;
    fetchUnit->instFetchBuffer = instFetchBuffer;
    fetchUnit->instFetchBufferSize = instFetchBufferSize;
    fetchUnit->numInstsInBuffer = numInstsInBuffer;
}

// free any elements of fetch unit that are stored on the heap
void teardownFetchUnit(FetchUnit *fetchUnit) {

}

// doubles the size of the output buffer if necessary
void extendFetchUnitOutputBufferIfNeeded(FetchUnit *fetchUnit) {

    // only extend the buffer if it is full
    if (*fetchUnit->numInstsInBuffer == *fetchUnit->instFetchBufferSize) {

        // get old values
        Instruction **oldBuffer = fetchUnit->instFetchBuffer;
        int oldSize = *fetchUnit->instFetchBufferSize;
        int newSize = oldSize * 2;

        // reallocate array and copy contents
        *fetchUnit->instFetchBufferSize = newSize;
        fetchUnit->instFetchBuffer = realloc(fetchUnit->instFetchBuffer, newSize * sizeof(Instruction *));

        for (int i = *fetchUnit->numInstsInBuffer; i < newSize; i++) {
            fetchUnit->instFetchBuffer[i] = NULL;
        }

        printf("extending fetch unit output buffer to %i entries\n", newSize);
    }
}

// adds a fetched instruction to the output buffer
void addInstToFetchUnitOutputBuffer(FetchUnit *fetchUnit, Instruction *inst) {
    extendFetchUnitOutputBufferIfNeeded(fetchUnit);

    fetchUnit->instFetchBuffer[(*fetchUnit->numInstsInBuffer)++] = inst;

    printf("added instruction: %p to fetch buffer, numInstsInBuffer: %i\n", inst, *fetchUnit->numInstsInBuffer);
}

// execute fetch unit's operations during a clock cycle
void cycleFetchUnit(FetchUnit *fetchUnit, RegisterFile *registerFile, InstCache *instCache) {

    printf("\nperforming fetch unit operations...\n");

    // get the current value of PC
    int pcVal = readRegisterFileInt(registerFile, PHYS_REG_PC);

    // get the next NF instructions
    for (int i = 0; i < fetchUnit->NF; i++) {

        // get the instruction from the instruction cache
        Instruction *inst = readInstructionCache(instCache, pcVal);
        if (!inst) {
            printf("could not get instruction from cache\n");
            break;
        }

        // write the instruction to the buffer
        addInstToFetchUnitOutputBuffer(fetchUnit, inst);

        pcVal += 4;
    }
    
    // update the new value of PC in the register file
    writeRegisterFileInt(registerFile, PHYS_REG_PC, pcVal);

    // printf("instructions in fetch unit output buffer: \n");
    // for (int i = 0; i < fetchUnit->numInstsInBuffer; i++) {
    //     printInstruction(*(fetchUnit->outputBuffer[i]));
    // }
 }