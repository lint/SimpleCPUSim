
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
    StatusTables *statusTables = malloc(sizeof(StatusTables));
    cpu->statusTables = statusTables;

    statusTables->robTable = malloc(sizeof(ROBStatusTable));
    initROBStatusTable(statusTables->robTable, params->NR);

    statusTables->resStationTable = malloc(sizeof(ResStationStatusTable));
    initResStationStatusTable(statusTables->resStationTable);

    statusTables->regTable = malloc(sizeof(RegisterStatusTable));
    initRegisterStatusTable(statusTables->regTable);

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

    cpu->writebackUnit = malloc(sizeof(WritebackUnit));
    initWritebackUnit(cpu->writebackUnit, params->NB, params->NR);

    /* initialize functional units */

    FunctionalUnits *fus = malloc(sizeof(FunctionalUnits));
    cpu->functionalUnits = fus;

    IntFunctionalUnit *intFU = malloc(sizeof(IntFunctionalUnit));
    initIntFunctionalUnit(intFU);
    fus->intFU = intFU;
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

    if (cpu->issueUnit) {
        teardownIssueUnit(cpu->issueUnit);
        free(cpu->issueUnit);
    }

    if (cpu->writebackUnit) {
        teardownWritebackUnit(cpu->writebackUnit);
        free(cpu->writebackUnit);
    }

    if (cpu->functionalUnits) {
        if (cpu->functionalUnits->intFU) {
            teardownIntFunctionalUnit(cpu->functionalUnits->intFU);
            free(cpu->functionalUnits->intFU);
        }

        free(cpu->functionalUnits);
    }

    if (cpu->statusTables) {
        
        if (cpu->statusTables->robTable) {
            teardownROBStatusTable(cpu->statusTables->robTable);
            free(cpu->statusTables->robTable);
        }

        if (cpu->statusTables->regTable) {
            teardownRegisterStatusTable(cpu->statusTables->regTable);
            free(cpu->statusTables->regTable);
        }

        if (cpu->statusTables->resStationTable) {
            teardownResStationStatusTable(cpu->statusTables->resStationTable);
            free(cpu->statusTables->resStationTable);
        }
        free(cpu->statusTables);
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

// helper method to print the contents of the all status tables
void printStatusTables(CPU *cpu) {
    printRegisterStatusTable(cpu->statusTables->regTable);
    printROBStatusTable(cpu->statusTables->robTable);
    printResStationStatusTable(cpu->statusTables->resStationTable);
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
    cycleIntFunctionalUnit(cpu->functionalUnits->intFU, cpu->statusTables);
    printIntFunctionalUnit(cpu->functionalUnits->intFU);
}

// start executing instructions on the CPU
void executeCPU(CPU *cpu) {

    int cycle = 0;

    // infinite loop to cycle the clock until execution finishes
    for (int i = 1; i <= 5; i++) {
        printf("\ncycle: %i\n", i);

        cycleWritebackUnit(cpu->writebackUnit, cpu->statusTables, cpu->functionalUnits);
        printWritebackUnitCDBS(cpu->writebackUnit);
        printStatusTables(cpu);

        cycleFunctionalUnits(cpu);

        cycleIssueUnit(cpu->issueUnit, cpu->statusTables, cpu->registerFile, cpu->stallStats);
        printDecodeUnitOutputQueue(cpu);
        printStatusTables(cpu);

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

    printRegisterFileArchRegs(cpu->registerFile, cpu->decodeUnit);

    printStallStats(cpu->stallStats);
}