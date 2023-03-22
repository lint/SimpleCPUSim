
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fetch_unit.h"
#include "../cpu.h"
#include "../memory/memory.h"
#include "../types/types.h"

// initialize the fetch unit struct
void initFetchUnit(FetchUnit *fetchUnit, int NF) {

    printf("initializing fetch unit...\n");

    fetchUnit->NF = NF;
    fetchUnit->outputBuffer = calloc(sizeof(Instruction *), NF);
    fetchUnit->outputBufferSize = NF;
    fetchUnit->numInstsInBuffer = 0;
}

// free any elements of fetch unit that are stored on the heap
void teardownFetchUnit(FetchUnit *fetchUnit) {
    if (fetchUnit->outputBuffer) {
        free(fetchUnit->outputBuffer);
    }
}

// doubles the size of the output buffer if necessary
void extendFetchUnitOutputBufferIfNeeded(FetchUnit *fetchUnit) {

    // only extend the buffer if it is full
    if (fetchUnit->numInstsInBuffer == fetchUnit->outputBufferSize) {
        
        // get old values
        Instruction **oldBuffer = fetchUnit->outputBuffer;
        int oldSize = fetchUnit->outputBufferSize;
        int newSize = oldSize * 2;

        // reallocate array and copy contents
        fetchUnit->outputBufferSize = newSize;
        fetchUnit->outputBuffer = calloc(newSize, sizeof(Instruction *));
        memcpy(fetchUnit->outputBuffer, oldBuffer, oldSize * sizeof(Instruction *));
        free(oldBuffer);

        printf("extending instruction cache to %i entries\n", newSize);
    }
}

// adds a fetched instruction to the output buffer
void addInstToFetchUnitOutputBuffer(FetchUnit *fetchUnit, Instruction *inst) {
    extendFetchUnitOutputBufferIfNeeded(fetchUnit);
    fetchUnit->outputBuffer[fetchUnit->numInstsInBuffer++] = inst;
}

// execute fetch unit's operations during a clock cycle
void cycleFetchUnit(FetchUnit *fetchUnit, RegisterFile *registerFile, InstCache *instCache) {

    // get the current value of PC
    int pcVal = readRegisterFile(registerFile, PHYS_PC);

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
    writeRegisterFile(registerFile, PHYS_PC, pcVal);

    // printf("instructions in fetch unit output buffer: \n");
    // for (int i = 0; i < fetchUnit->numInstsInBuffer; i++) {
    //     printInstruction(*(fetchUnit->outputBuffer[i]));
    // }
 }