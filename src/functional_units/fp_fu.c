
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../status_tables/status_tables.h"
#include "../misc/misc.h"
#include "fp_fu.h"

// initialize a floating point functional unit
void initFPFunctionalUnit(FPFunctionalUnit *fpFU, enum FunctionalUnitType fuType, int latency) {
    fpFU->latency = latency;
    fpFU->lastSelectedResStation = -1;
    fpFU->fuType = fuType;
    fpFU->isStalled = 0;
    fpFU->stages = malloc(fpFU->latency * sizeof(IntFUResult *)); 
    for (int i = 0; i < fpFU->latency; i++) {
        fpFU->stages[i] = NULL;
    }
}

// free any elements of a floating point functional unit that are stored on the heap
void teardownFPFunctionalUnit(FPFunctionalUnit *fpFU) {
    if (fpFU->stages) {
        free(fpFU->stages);
    }
}

// gets the last element in the stages array, which indicates the most recent completed result
FloatFUResult *getCurrentFPFunctionalUnitResult(FPFunctionalUnit *fpFU) {
    return fpFU->stages[fpFU->latency-1];
}

// helper method to print the contents of the fp functional unit
void printFPFunctionalUnit(FPFunctionalUnit *fpFU) {
    printf("fp functional unit: fuType: %s, latency: %i lastSelectedResStation: %i isStalled: %i\n", fuTypeToString(fpFU->fuType), fpFU->latency, fpFU->lastSelectedResStation, fpFU->isStalled);

    for (int i = 0; i < fpFU->latency; i++) {
        printf("\tstage: %i, ", i);
        FloatFUResult *stageElement = fpFU->stages[i];
        if (!stageElement) {
            printf("NULL\n");
        } else {
            printf("entry: %p, source1: %f, source2: %f, result: %f, destROB: %i\n", 
                stageElement, stageElement->source1, stageElement->source2, stageElement->result, stageElement->destROB);
        }
    }
}

// removes the content of the the int functional unit
void flushFPFunctionalUnit(FPFunctionalUnit *fpFU) {
    
    for (int i = 0; i < fpFU->latency; i++) {
        fpFU->stages[i] = NULL;
    }

    fpFU->isStalled = 0;
}

// perform fp functional unit operations during a cycle
void cycleFPFunctionalUnit(FPFunctionalUnit *fpFU, StatusTables *statusTables) {
    #ifdef ENABLE_DEBUG_LOG
    printf("\nperforming fp functional unit (%s) operations...\n", fuTypeToString(fpFU->fuType));
    #endif

    if (fpFU->isStalled) {
        #ifdef ENABLE_DEBUG_LOG
        printf("\tfp functional unit (%s) is stalled because its result was not placed on the CDB by the writeback unit\n", fuTypeToString(fpFU->fuType));
        #endif

        return;
    }

    ResStationStatusTable *resStationTable = statusTables->resStationTable;
    ROBStatusTable *robTable = statusTables->robTable;

    ResStationStatusTableEntry **resStationEntries = resStationEntriesForFunctionalUnit(resStationTable, fpFU->fuType);
    int numResStations = numResStationsForFunctionalUnit(resStationTable, fpFU->fuType);
    int nextResStation = (fpFU->lastSelectedResStation + 1) % numResStations;
    FloatFUResult *nextResult = NULL;

    // iterate over all reservation stations to find the next operands to operate on
    for (int i = 0; i < numResStations; i++) {

        // get the reservation station entry's destination ROB and use it to get the assocaited ROB entry
        ResStationStatusTableEntry *resStationEntry = resStationEntries[nextResStation];
        int destROB = resStationEntry->dest;
        ROBStatusTableEntry *robEntry = robTable->entries[destROB];

        // do not allow instructions that just received a value from the CDB to execute in the same cycle
        if (resStationEntry->busy && resStationEntry->justGotOperandFromCDB) {
            resStationEntry->justGotOperandFromCDB = 0;
            nextResStation = (nextResStation + 1) % numResStations;
            continue;
        }

        // if both operands of the reservation station and the instruction's state is "issued" then it can be brought into the functional unit
        if (resStationEntry->busy && (resStationEntry->vjIsAvailable && resStationEntry->vkIsAvailable) && robEntry->state == INST_STATE_ISSUED) {

            #ifdef ENABLE_DEBUG_LOG
            printf("selecting reservation station: %s[%d] for execution\n", fuTypeToString(fpFU->fuType), resStationEntry->resStationIndex);
            #endif
            
            fpFU->lastSelectedResStation = nextResStation;

            // update entry in ROB to "executing"
            robEntry->state = INST_STATE_EXECUTING;

            // allocate and initialize the next result which will get passed through the stages of the functional unit
            nextResult = malloc(sizeof(FloatFUResult));
            nextResult->source1 = resStationEntry->vjFloat;
            nextResult->source2 = resStationEntry->vkFloat;
            nextResult->destROB = destROB;

            // perform different calculations based on functional unit type and operation
            if (fpFU->fuType == FU_TYPE_FPADD) {

                if (resStationEntry->op == FU_OP_ADD) {
                    nextResult->result = nextResult->source1 + nextResult->source2;
                } else if (resStationEntry->op == FU_OP_SUB) {
                    nextResult->result = nextResult->source1 - nextResult->source2;    
                } else {
                    #ifdef ENABLE_DEBUG_LOG
                    printf("error: invalid operation type: %s for FPAdd functional unit\n", fuOpToString(resStationEntry->op));
                    #endif
                }

            } else if (fpFU->fuType == FU_TYPE_FPMUL) {
                nextResult->result = nextResult->source1 * nextResult->source2;
            } else if (fpFU->fuType == FU_TYPE_FPDIV) {
                nextResult->result = nextResult->source1 / nextResult->source2;
            } else {
                printf("error: tried to start executing an instruction in the fp functional unit with an invalid functional unit type: %s\n", fuTypeToString(fpFU->fuType));
                exit(1);
            }

            break;
        }
        
        nextResStation = (nextResStation + 1) % numResStations;
    }

    #ifdef ENABLE_DEBUG_LOG
    if (!nextResult) {
        printf("no reservation station found to start executing\n");
    }
    #endif

    // move data through the stages of the functional unit by shifting elements of the stages array to the right
    for (int i = fpFU->latency - 1; i >= 1; i--) {
        fpFU->stages[i] = fpFU->stages[i - 1];
    }

    // move the next result into the first stage element
    fpFU->stages[0] = nextResult;

    // forward most recently completed result to reservation stations
    FloatFUResult *newestResult = getCurrentFPFunctionalUnitResult(fpFU);
    if (newestResult) {
        sendFloatUpdateToResStationStatusTable(resStationTable, newestResult->destROB, newestResult->result, 0);
    }
}