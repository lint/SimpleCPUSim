
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

    cpu->cycle = 1;
    cpu->consecEmptyROBCycles = 0;

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

    // initialize instruction fetch buffer that is shared between fetch and decode units
    Instruction **instFetchBuffer = calloc(params->NF, sizeof(Instruction *));
    int *instFetchBufferSize = malloc(sizeof(int));
    int *numInstsInFetchBuffer = malloc(sizeof(int));
    *instFetchBufferSize = params->NF;
    *numInstsInFetchBuffer = 0;

    cpu->fetchUnit = malloc(sizeof(FetchUnit));
    initFetchUnit(cpu->fetchUnit, params->NF, instFetchBuffer, instFetchBufferSize, numInstsInFetchBuffer);

    cpu->decodeUnit = malloc(sizeof(DecodeUnit));
    initDecodeUnit(cpu->decodeUnit, params->NI, params->NW, instFetchBuffer, instFetchBufferSize, numInstsInFetchBuffer);

    cpu->writebackUnit = malloc(sizeof(WritebackUnit));
    initWritebackUnit(cpu->writebackUnit, params->NB, params->NR);

    /* initialize functional units */

    FunctionalUnits *fus = malloc(sizeof(FunctionalUnits));
    cpu->functionalUnits = fus;

    IntFunctionalUnit *intFU = malloc(sizeof(IntFunctionalUnit));
    initIntFunctionalUnit(intFU, 1); // latency decribed in project description
    fus->intFU = intFU;

    FPFunctionalUnit *fpAddFU = malloc(sizeof(FPFunctionalUnit));
    initFPFunctionalUnit(fpAddFU, FU_TYPE_FPADD, 3); // latency decribed in project description
    fus->fpAddFU = fpAddFU;

    FPFunctionalUnit *fpMulFU = malloc(sizeof(FPFunctionalUnit));
    initFPFunctionalUnit(fpMulFU, FU_TYPE_FPMUL, 4); // latency decribed in project description
    fus->fpMulFU = fpMulFU;

    FPFunctionalUnit *fpDivFU = malloc(sizeof(FPFunctionalUnit));
    initFPFunctionalUnit(fpDivFU, FU_TYPE_FPDIV, 8); // latency decribed in project description
    fus->fpDivFU = fpDivFU;
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

    if (cpu->fetchUnit->instFetchBuffer) {
        free(cpu->fetchUnit->instFetchBuffer);
    }

    if (cpu->fetchUnit) {
        teardownFetchUnit(cpu->fetchUnit);
        free(cpu->fetchUnit);
    }

    if (cpu->decodeUnit) {
        teardownDecodeUnit(cpu->decodeUnit);
        free(cpu->decodeUnit);
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
        if (cpu->functionalUnits->fpAddFU) {
            teardownFPFunctionalUnit(cpu->functionalUnits->fpAddFU);
            free(cpu->functionalUnits->fpAddFU);
        }
        if (cpu->functionalUnits->fpMulFU) {
            teardownFPFunctionalUnit(cpu->functionalUnits->fpMulFU);
            free(cpu->functionalUnits->fpMulFU);
        }
        if (cpu->functionalUnits->fpDivFU) {
            teardownFPFunctionalUnit(cpu->functionalUnits->fpDivFU);
            free(cpu->functionalUnits->fpDivFU);
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

    cycleFPFunctionalUnit(cpu->functionalUnits->fpAddFU, cpu->statusTables);
    printFPFunctionalUnit(cpu->functionalUnits->fpAddFU);

    cycleFPFunctionalUnit(cpu->functionalUnits->fpMulFU, cpu->statusTables);
    printFPFunctionalUnit(cpu->functionalUnits->fpMulFU);

    cycleFPFunctionalUnit(cpu->functionalUnits->fpDivFU, cpu->statusTables);
    printFPFunctionalUnit(cpu->functionalUnits->fpDivFU);
}

// checks to see if the ROB status table is empty, indicating that the program has finished execution
int executionIsComplete(CPU *cpu) {

    // 2 cycles are required for the first instruction be issued and assigned a ROB entry
    if (cpu->cycle <= 2) {
        return 0;
    // emergency cut off if cpu has reached one billion cycles
    } else if (cpu->cycle >= 1000000000) {
        return 1;
    }
    
    // track the number of cycles that the ROB is consecutively empty
    if (isROBEmpty(cpu->statusTables->robTable)) {
        cpu->consecEmptyROBCycles++; 
    } else {
        cpu->consecEmptyROBCycles = 0;
    }

    // after an incorrect branch prediction flushes the ROB, it takes 2 cycles to fetch and issue the correct instruction
    // if the ROB is still empty after this, then execution must be complete
    return cpu->consecEmptyROBCycles > 2;
}

// start executing instructions on the CPU
void executeCPU(CPU *cpu) {

    // infinite loop to cycle the clock until execution finishes
    // for (int i = 1; i <= 5; i++) {
    while (!executionIsComplete(cpu)) {

        printf("\n----------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
        printf("\ncycle: %i\n", cpu->cycle);

        cycleWritebackUnit(cpu->writebackUnit, cpu->decodeUnit, cpu->statusTables, cpu->functionalUnits, cpu->registerFile);
        printWritebackUnitCDBS(cpu->writebackUnit);
        printStatusTables(cpu);

        cycleFunctionalUnits(cpu);

        //cycleIssueUnit(cpu->issueUnit, cpu->statusTables, cpu->registerFile, cpu->stallStats);
        printStatusTables(cpu);

        cycleDecodeUnit(cpu->decodeUnit, cpu->statusTables, cpu->registerFile, cpu->stallStats);
        printFreeList(cpu->decodeUnit);
        printMapTable(cpu->decodeUnit);
        printDecodeQueue(cpu->decodeUnit);
        printInstructionFetchBuffer(cpu);
        printStatusTables(cpu);

        cycleFetchUnit(cpu->fetchUnit, cpu->registerFile, cpu->instCache);
        printInstructionFetchBuffer(cpu);
        
        // break;
        cpu->cycle++;
    }

    printf("executed cycles: %i\n", cpu->cycle);
    printRegisterFile(cpu->registerFile);
    printStallStats(cpu->stallStats);

    teardownCPU(cpu);
}