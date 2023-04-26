
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "misc/misc.h"
#include "memory/memory.h"
#include "stage_units/stage_units.h"
#include "status_tables/status_tables.h"
#include "functional_units/functional_units.h"
#include "branch_prediction/branch_predictor.h"

#include "cpu.h"

// create new stall statistics struct
void initStallStats(StallStats *stallStats) {
    stallStats->fullResStationStalls = 0;
    stallStats->fullROBStalls = 0;
}

// creates a new CPU struct, initializes its data structures and components, and returns it
void initCPU(CPU *cpu, Params *params) {

    //tracks overall cycles executed and execution completion detection respectively
    cpu->cycle = 1;
    cpu->consecEmptyROBCycles = 0;

    // store input parameters
    cpu->params = params;

    /* initialize helper structs */

    cpu->stallStats = malloc(sizeof(StallStats));
    initStallStats(cpu->stallStats);

    cpu->labelTable = malloc(sizeof(LabelTable));
    initLabelTable(cpu->labelTable);
   
    // initialize data and instruction memory caches
    cpu->dataCache = malloc(sizeof(DataCache));
    initDataCache(cpu->dataCache);

    cpu->instCache = malloc(sizeof(InstCache));
    initInstCache(cpu->instCache);

    // initialize register file
    cpu->registerFile = malloc(sizeof(RegisterFile));
    initRegisterFile(cpu->registerFile);

    /* initialize status tables */
    
    StatusTables *statusTables = malloc(sizeof(StatusTables));
    cpu->statusTables = statusTables;

    statusTables->robTable = malloc(sizeof(ROBStatusTable));
    initROBStatusTable(statusTables->robTable, params->NR);

    statusTables->resStationTable = malloc(sizeof(ResStationStatusTable));
    initResStationStatusTable(statusTables->resStationTable);

    statusTables->regTable = malloc(sizeof(RegisterStatusTable));
    initRegisterStatusTable(statusTables->regTable);

    /* initialize stage units */

    cpu->fetchUnit = malloc(sizeof(FetchUnit));
    initFetchUnit(cpu->fetchUnit, params->NF);

    cpu->decodeUnit = malloc(sizeof(DecodeUnit));
    initDecodeUnit(cpu->decodeUnit, params->NI, params->NW);

    cpu->memUnit = malloc(sizeof(MemoryUnit));
    initMemoryUnit(cpu->memUnit);

    cpu->writebackUnit = malloc(sizeof(WritebackUnit));
    initWritebackUnit(cpu->writebackUnit, params->NB, params->NR);

    /* initialize functional units */

    // bundles the functional units into one object
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

    BUFunctionalUnit *buFU = malloc(sizeof(BUFunctionalUnit));
    initBUFunctionalUnit(buFU, 1); // latency described in project description
    fus->buFU = buFU;

    LSFunctionalUnit *lsFU = malloc(sizeof(LSFunctionalUnit));
    initLSFunctionalUnit(lsFU, 1); // latency described in project description
    fus->lsFU = lsFU;
    
    // initialize branch predictor
    cpu->branchPredictor = malloc(sizeof(BranchPredictor));
    initBranchPredictor(cpu->branchPredictor);
}

// free any elements of the CPU that were stored on the heap
void teardownCPU(CPU *cpu) {

    /* free helper structs */

    if (cpu->stallStats) {
        free(cpu->stallStats);
    }

    if (cpu->labelTable) {
        teardownLabelTable(cpu->labelTable);
        free(cpu->labelTable);
    }

    /* free caches */

    if (cpu->dataCache) {
        teardownDataCache(cpu->dataCache);
        free(cpu->dataCache);
    }

    if (cpu->instCache) {
        teardownInstCache(cpu->instCache);
        free(cpu->instCache);
    }

    // free register file
    if (cpu->registerFile) {
        teardownRegisterFile(cpu->registerFile);
        free(cpu->registerFile);
    }

    /* free stage units */

    if (cpu->fetchUnit) {
        teardownFetchUnit(cpu->fetchUnit);
        free(cpu->fetchUnit);
    }

    if (cpu->decodeUnit) {
        teardownDecodeUnit(cpu->decodeUnit);
        free(cpu->decodeUnit);
    }

    if (cpu->memUnit) {
        teardownMemoryUnit(cpu->memUnit);
        free(cpu->memUnit);
    }

    if (cpu->writebackUnit) {
        teardownWritebackUnit(cpu->writebackUnit);
        free(cpu->writebackUnit);
    }

    // free functional units
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
        if (cpu->functionalUnits->lsFU) {
            teardownLSFunctionalUnit(cpu->functionalUnits->lsFU);
            free(cpu->functionalUnits->lsFU);
        }

        free(cpu->functionalUnits);
    }

    // free status tables
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

    // free branch predictor
    if (cpu->branchPredictor) {
        teardownBranchPredictor(cpu->branchPredictor);
        free(cpu->branchPredictor);
    }
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
    #ifdef ENABLE_DEBUG_LOG
    printf("\nperforming functional unit operations...\n");
    #endif

    // execute INT functional unit operations
    cycleIntFunctionalUnit(cpu->functionalUnits->intFU, cpu->statusTables);
    #ifdef ENABLE_DEBUG_LOG
    printIntFunctionalUnit(cpu->functionalUnits->intFU);
    #endif

    // execute FPAdd functional unit operations
    cycleFPFunctionalUnit(cpu->functionalUnits->fpAddFU, cpu->statusTables);
    #ifdef ENABLE_DEBUG_LOG
    printFPFunctionalUnit(cpu->functionalUnits->fpAddFU);
    #endif

    // execute FPMul functional unit operations
    cycleFPFunctionalUnit(cpu->functionalUnits->fpMulFU, cpu->statusTables);
    #ifdef ENABLE_DEBUG_LOG
    printFPFunctionalUnit(cpu->functionalUnits->fpMulFU);
    #endif

    // execute FPDiv functional unit operations
    cycleFPFunctionalUnit(cpu->functionalUnits->fpDivFU, cpu->statusTables);
    #ifdef ENABLE_DEBUG_LOG
    printFPFunctionalUnit(cpu->functionalUnits->fpDivFU);
    #endif

    // execute branch unit functional unit operations
    cycleBUFunctionalUnit(cpu->functionalUnits->buFU, cpu->statusTables);
    #ifdef ENABLE_DEBUG_LOG
    printBUFunctionalUnit(cpu->functionalUnits->buFU);
    #endif

    // execute load/store functional unit operations
    cycleLSFunctionalUnit(cpu->functionalUnits->lsFU, cpu->statusTables);
    #ifdef ENABLE_DEBUG_LOG
    printLSFunctionalUnit(cpu->functionalUnits->lsFU);
    #endif
}

// checks to see if the ROB status table is empty, indicating that the program has finished execution
int executionIsComplete(CPU *cpu) {

    // 2 cycles are required for the first instruction be issued and assigned a ROB entry
    if (cpu->cycle <= 2) {
        return 0;
    // emergency cut off if cpu has reached some maximum number of cycles
    } else if (cpu->cycle >= 1000000) {
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
    return cpu->consecEmptyROBCycles >= 2;
}

// start executing instructions on the CPU
void executeCPU(CPU *cpu) {

    // infinite loop to cycle the clock until execution finishes
    while (!executionIsComplete(cpu)) {

       
        // perform writeback unit operations
        cycleWritebackUnit(cpu->writebackUnit, cpu->fetchUnit, cpu->decodeUnit, cpu->memUnit, 
            cpu->statusTables, cpu->functionalUnits, cpu->registerFile, cpu->dataCache, cpu->branchPredictor);

        // perform memory unit operations
        cycleMemoryUnit(cpu->memUnit, cpu->dataCache, cpu->functionalUnits->lsFU, cpu->statusTables);
        
        // perform functional unit operations
        cycleFunctionalUnits(cpu);

        // perform decode unit operations
        cycleDecodeUnit(cpu->decodeUnit, cpu->fetchUnit->fetchBuffer, cpu->fetchUnit->numInstsInBuffer, cpu->statusTables, cpu->registerFile, cpu->stallStats, cpu->labelTable);
        
        // perform fetch unit operations
        cycleFetchUnit(cpu->fetchUnit, cpu->registerFile, cpu->instCache, cpu->branchPredictor);
        
        // print debug information if enabled
        #ifdef ENABLE_DEBUG_LOG
        printf("\n----------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
        printf("\ncycle: %i\n\n", cpu->cycle);
        printWritebackUnitCDBs(cpu->writebackUnit);
        printMemoryUnit(cpu->memUnit);
        printFreeList(cpu->decodeUnit);
        printMapTable(cpu->decodeUnit);
        printDecodeQueue(cpu->decodeUnit);
        printInstructionFetchBuffer(cpu->fetchUnit);
        printStatusTables(cpu);
        printBranchPredictor(cpu->branchPredictor);
        printRegisterFile(cpu->registerFile);
        printMapTable(cpu->decodeUnit);
        #endif

        cpu->cycle++;
    }

    printf("\n -- DONE EXECUTION --\n\n");
    printRegisterFile(cpu->registerFile);
    printf("\n");
    printDataCache(cpu->dataCache);
    printf("\n");
    printStallStats(cpu->stallStats);
    printf("\nexecuted cycles: %i\n", cpu->cycle);

    teardownCPU(cpu);
}