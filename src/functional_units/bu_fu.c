
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../status_tables/status_tables.h"
#include "../misc/misc.h"
#include "bu_fu.h"

// initialize a BU functional unit struct
void initBUFunctionalUnit(BUFunctionalUnit *buFU, int latency) {
    buFU->latency = latency;
    buFU->lastSelectedResStation = -1;
    buFU->fuType = FU_TYPE_BU;
    buFU->isStalled = 0;
    buFU->stages = malloc(buFU->latency * sizeof(BUFUResult *)); 
    buFU->stages[0] = NULL;
}

// free any elements of the BU functional unit that are stored on the heap
void teardownBUFunctionalUnit(BUFunctionalUnit *buFU) {
    if (buFU->stages) {
        free(buFU->stages);
    }
}

// gets the last element in the stages array, which indicates the most recent completed result
BUFUResult *getCurrentBUFunctionalUnitResult(BUFunctionalUnit *buFU) {
    return buFU->stages[buFU->latency-1];
}

// helper method to print the contents of the BU functional unit
void printBUFunctionalUnit(BUFunctionalUnit *buFU) {
    printf("BU functional unit: latency: %i lastSelectedResStation: %i isStalled: %i\n", buFU->latency, buFU->lastSelectedResStation, buFU->isStalled);

    for (int i = 0; i < buFU->latency; i++) {
        printf("\tstage: %i, ", i);
        
        BUFUResult *stageElement = buFU->stages[i];

        if (!stageElement) {
            printf("NULL\n");
        } else {
            printf("entry: %p, source1: %i, source2: %i, isBranchTaken: %i, effective address: %i\n", 
                stageElement, stageElement->source1, stageElement->source2, stageElement->isBranchTaken, stageElement->effAddr);
        }
    }
}

// removes the content of the the int functional unit
void flushBUFunctionalUnit(BUFunctionalUnit *buFU) {
    
    for (int i = 0; i < buFU->latency; i++) {
        buFU->stages[i] = NULL;
    }

    buFU->isStalled = 0;
}

// perform BU functional unit operations over the course of a clock cycle
void cycleBUFunctionalUnit(BUFunctionalUnit *buFU, StatusTables *statusTables) {
    printf_DEBUG(("\nperforming BU functional unit operations...\n"));

    if (buFU->isStalled) {
        printf_DEBUG(("\nBU functional unit is stalled because its result was not placed on the CDB by the writeback unit\n"));
        return;
    }

    ResStationStatusTable *resStationTable = statusTables->resStationTable;
    ROBStatusTable *robTable = statusTables->robTable;

    ResStationStatusTableEntry **resStationEntries = resStationEntriesForFunctionalUnit(resStationTable, buFU->fuType);
    int numResStations = numResStationsForFunctionalUnit(resStationTable, buFU->fuType);
    int nextResStation = (buFU->lastSelectedResStation + 1) % numResStations;
    BUFUResult *nextResult = NULL;

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
            printf("selecting reservation station: BU[%d] for execution\n", resStationEntry->resStationIndex);
            #endif
            
            buFU->lastSelectedResStation = nextResStation;

            // update entry in ROB to "executing"
            robEntry->state = INST_STATE_EXECUTING;

            // allocate and initialize the next result which will get passed through the stages of the functional unit
            nextResult = malloc(sizeof(BUFUResult));
            nextResult->source1 = resStationEntry->vjInt;
            nextResult->source2 = resStationEntry->vkInt;
            nextResult->destROB = destROB;
            
            // take branch if operands are not equal
            if (nextResult->source1 != nextResult->source2) {
                
                nextResult->isBranchTaken = 1;
                nextResult->effAddr = resStationEntry->addr + resStationEntry->buOffset;

                #ifdef ENABLE_DEBUG_LOG
                printf("\nresStation->addr: %i, buOffset: %i\n", resStationEntry->addr, resStationEntry->buOffset);
                #endif

            // do not take branch if operands are equal
            } else {

                nextResult->isBranchTaken = 0;
                nextResult->effAddr = resStationEntry->addr + 4;
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
    // this does not do anything given the project design as the stages array is only one element, so it's commented out, but it's good to be general
    // for (int i = buFU->latency - 1; i >= 1; i--) {
    //     buFU->stages[i] = buFU->stages[i - 1];
    // }

    // move the next result into the first stage element
    buFU->stages[0] = nextResult;
}