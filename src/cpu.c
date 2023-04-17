
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cpu.h"
#include "types/types.h"
#include "memory/memory.h"
#include "stage_units/stage_units.h"
#include "status_tables/status_tables.h"
#include "functional_units/functional_units.h"

// create new stall statistics struct
void initStallStats(StallStats *stallStats) {
    stallStats->fullResStationStalls = 0;
    stallStats->fullROBStalls = 0;
}

// creates a new CPU struct, initializes its data structures and components, and returns it
void initCPU(CPU *cpu, Params *params) {
    cpu->params = params;

    cpu->stallStats = malloc(sizeof(StallStats));
    initStallStats(cpu->stallStats);
   
    // initialize data and instruction memory caches
    cpu->dataCache = malloc(sizeof(DataCache));
    initDataCache(cpu->dataCache);

    cpu->instCache = malloc(sizeof(InstCache));
    initInstCache(cpu->instCache);

    // initialize register file
    cpu->registerFile = malloc(sizeof(RegisterFile));
    initRegisterFile(cpu->registerFile);

    // initialize status tables
    cpu->robTable = malloc(sizeof(ROBStatusTable));
    initROBStatusTable(cpu->robTable, params->NR);

    cpu->resStationTable = malloc(sizeof(ResStationStatusTable));
    initResStationStatusTable(cpu->resStationTable);

    cpu->regTable = malloc(sizeof(RegisterStatusTable));
    initRegisterStatusTable(cpu->regTable);

    /* initialize stage units */

    // initialize instruction fetch buffer
    Instruction **instFetchBuffer = calloc(params->NF, sizeof(Instruction *));
    int *instFetchBufferSize = malloc(sizeof(int));
    int *numInstsInFetchBuffer = malloc(sizeof(int));
    *instFetchBufferSize = params->NF;
    *numInstsInFetchBuffer = 0;

    // initalize decode queue
    Instruction **instDecodeQueue = calloc(params->NI, sizeof(Instruction *));
    int *numInstsInDecodeQueue = malloc(sizeof(int));    

    cpu->fetchUnit = malloc(sizeof(FetchUnit));
    initFetchUnit(cpu->fetchUnit, params->NF, instFetchBuffer, instFetchBufferSize, numInstsInFetchBuffer);

    cpu->decodeUnit = malloc(sizeof(DecodeUnit));
    initDecodeUnit(cpu->decodeUnit, params->NF, params->NI, instFetchBuffer, instFetchBufferSize, numInstsInFetchBuffer, instDecodeQueue, numInstsInDecodeQueue);

    cpu->issueUnit = malloc(sizeof(IssueUnit));
    initIssueUnit(cpu->issueUnit, params->NW, params->NI, instDecodeQueue, numInstsInDecodeQueue);

    /* initialize functional units */

    cpu->intFU = malloc(sizeof(IntFunctionalUnit));
    initIntFunctionalUnit(cpu->intFU);
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

    if (cpu->robTable) {
        teardownROBStatusTable(cpu->robTable);
        free(cpu->robTable);
    }

    if (cpu->regTable) {
        teardownRegisterStatusTable(cpu->regTable);
        free(cpu->regTable);
    }

    if (cpu->resStationTable) {
        teardownResStationStatusTable(cpu->resStationTable);
        free(cpu->resStationTable);
    }

    if (cpu->fetchUnit) {
        teardownFetchUnit(cpu->fetchUnit);
        free(cpu->fetchUnit);
    }

    if (cpu->decodeUnit) {
        teardownDecodeUnit(cpu->decodeUnit);
        free(cpu->decodeUnit);
    }

    if (cpu->issueUnit) {
        teardownIssueUnit(cpu->issueUnit);
        free(cpu->issueUnit);
    }

    if (cpu->intFU) {
        teardownIntFunctionalUnit(cpu->intFU);
        free(cpu->intFU);
    }

    // make sure to free fetch buffer and decode queue
}

// helper method to print the contents of the instruction fetch buffer
void printInstructionFetchBuffer(CPU *cpu) {

    FetchUnit *fetchUnit = cpu->fetchUnit;

    printf("instruction fetch buffer: %p, size: %i, numInsts: %i, items: ", fetchUnit->instFetchBuffer, *fetchUnit->instFetchBufferSize, *fetchUnit->numInstsInBuffer);

    for (int i = 0; i < *fetchUnit->numInstsInBuffer; i++) {
        printf("%p, ", fetchUnit->instFetchBuffer[i]);
    }

    printf("\n");
}

// helper method to print the current state of the decoded instruction queue
void printDecodeUnitOutputQueue(CPU *cpu) {

    DecodeUnit *decodeUnit = cpu->decodeUnit;

    printf("instruction decode queue: %p, size: %i, numInsts: %i, items: ", decodeUnit->instDecodeQueue, decodeUnit->NI, *decodeUnit->numInstsInQueue);

    for (int i = 0; i < *decodeUnit->numInstsInQueue; i++) {
        printf("%p, ", decodeUnit->instDecodeQueue[i]);
        //printInstruction(*decodeUnit->instDecodeQueue[i]);
    }

    printf("\n");
}

// helper method to print the current state of the number of tracked stalls
void printStallStats(StallStats *stallStats) {
    printf("\nstall statistics:\n");
    printf("\tstalls due to full ROB: %i\n", stallStats->fullROBStalls);
    printf("\tstalls due to full reservation stations: %i\n", stallStats->fullResStationStalls);
}

// perform cycle operations for each functional unit
void cycleFunctionalUnits(CPU *cpu) {

    printf("\nperforming functional unit operations...\n");
    cycleIntFunctionalUnit(cpu->intFU, cpu->resStationTable, cpu->robTable);
    printIntFunctionalUnit(cpu->intFU);
}

// start executing instructions on the CPU
void executeCPU(CPU *cpu) {

    int cycle = 0;

    // infinite loop to cycle the clock until execution finishes
    for (int i = 0; i < 4; i++) {
        printf("\ncycle: %i\n", i);

        cycleFunctionalUnits(cpu);

        cycleIssueUnit(cpu->issueUnit, cpu->registerFile, cpu->robTable, cpu->resStationTable, cpu->regTable, cpu->stallStats);
        printDecodeUnitOutputQueue(cpu);
        printRegisterStatusTable(cpu->regTable);
        printROBStatusTable(cpu->robTable);
        printResStationStatusTable(cpu->resStationTable);

        cycleDecodeUnit(cpu->decodeUnit);
        printFreeList(cpu->decodeUnit);
        printMapTable(cpu->decodeUnit);
        printInstructionFetchBuffer(cpu);
        printDecodeUnitOutputQueue(cpu);

        cycleFetchUnit(cpu->fetchUnit, cpu->registerFile, cpu->instCache);
        printInstructionFetchBuffer(cpu);
        
        // break;
        cycle++;
    }

    printStallStats(cpu->stallStats);
}