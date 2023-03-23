
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cpu.h"
#include "types/types.h"
#include "memory/memory.h"
#include "stage_units/stage_units.h"

// creates a new CPU struct, initializes its data structures and components, and returns it
void initCPU(CPU *cpu, Params *params) {
    cpu->params = params;
   
    cpu->dataCache = malloc(sizeof(DataCache));
    initDataCache(cpu->dataCache);

    cpu->instCache = malloc(sizeof(InstCache));
    initInstCache(cpu->instCache);

    cpu->registerFile = malloc(sizeof(RegisterFile));
    initRegisterFile(cpu->registerFile);

    cpu->fetchUnit = malloc(sizeof(FetchUnit));
    initFetchUnit(cpu->fetchUnit, params->NF);

    cpu->decodeUnit = malloc(sizeof(DecodeUnit));
    initDecodeUnit(cpu->decodeUnit, params->NF, params->NI);
}

// free any elements of the CPU that were stored on the heap
void teardownCPU(CPU *cpu) {

    if (cpu->dataCache) {
        teardownDataCache(cpu->dataCache);
        free(cpu->dataCache);
    }

    if (cpu->instCache) {
        teardownInstCache(cpu->instCache);
        free(cpu->instCache);
    }

    if (cpu->registerFile) {
        teardownRegisterFile(cpu->registerFile);
        free(cpu->registerFile);
    }

    if (cpu->fetchUnit) {
        teardownFetchUnit(cpu->fetchUnit);
        free(cpu->fetchUnit);
    }

    if (cpu->decodeUnit) {
        teardownDecodeUnit(cpu->decodeUnit);
        free(cpu->decodeUnit);
    }
}

// sync the fetch and decode units' respective output and input buffers after cycle operations have completed
void syncFetchDecodeBuffer(CPU *cpu) {

    FetchUnit *fetchUnit = cpu->fetchUnit;
    DecodeUnit *decodeUnit = cpu->decodeUnit;
    int numInstsMoved = decodeUnit->numInstsMovedToQueue;

    // remove instructions from the buffer that have been added to the queue
    for (int i = 0; i < numInstsMoved; i++) {
        fetchUnit->outputBuffer[i] = NULL;
    }

    // shift remaining instructions to beginning of buffer
    if (numInstsMoved > 0) {
        for (int i = numInstsMoved; i < fetchUnit->numInstsInBuffer ;i++) {
            fetchUnit->outputBuffer[i - numInstsMoved] = fetchUnit->outputBuffer[i];
            fetchUnit->outputBuffer[i] = NULL;
        }
    }    

    fetchUnit->numInstsInBuffer -= numInstsMoved;

    // ensure the buffers are the same size
    if (decodeUnit->inputBufferSize < fetchUnit->outputBufferSize) {
        setDecodeUnitInputBufferSize(decodeUnit, fetchUnit->outputBufferSize);
    }

    // copy fetch output buffer to decode input buffer
    memcpy(decodeUnit->inputBuffer, fetchUnit->outputBuffer, fetchUnit->outputBufferSize * sizeof(Instruction *));
    decodeUnit->numInstsInBuffer = fetchUnit->numInstsInBuffer;
}

// start executing instructions on the CPU
void executeCPU(CPU *cpu) {

    int cycle = 0;

    // infinite loop to cycle the clock until execution finishes
    for (int i = 0; i < 3; i++) {
        printf("cycle: %i\n", i);

        cycleFetchUnit(cpu->fetchUnit, cpu->registerFile, cpu->instCache);
        cycleDecodeUnit(cpu->decodeUnit);
        
        syncFetchDecodeBuffer(cpu);

        printFetchUnitOutputBuffer(cpu->fetchUnit);
        printDecodeUnitInputBuffer(cpu->decodeUnit);
        printDecodeUnitOutputQueue(cpu->decodeUnit);
        // break;
        cycle++;
    }
}