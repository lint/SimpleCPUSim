
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../status_tables/status_tables.h"
#include "../misc/misc.h"
#include "ls_fu.h"

// initialize a INT functional unit struct
void initLSFunctionalUnit(LSFunctionalUnit *lsFU, int latency) {
    lsFU->latency = latency;
    lsFU->stages = malloc(lsFU->latency * sizeof(LSFUResult *)); 
    lsFU->stages[0] = NULL;
    lsFU->isStalled = 0;
}

// free any load/store functional unit elements that are stored on the heap
void teardownLSFunctionalUnit(LSFunctionalUnit *lsFU) {
    if (lsFU->stages) {
        free(lsFU->stages);
    }
}

// gets the last element in the stages array, which indicates the most recent completed result
LSFUResult *getCurrentLSFunctionalUnitResult(LSFunctionalUnit *lsFU) {
    return lsFU->stages[lsFU->latency-1];
}

// helper method to print the contents of the int functional unit
void printLSFunctionalUnit(LSFunctionalUnit *lsFU) {
    printf("int functional unit: latency: %i isStalled: %i\n", lsFU->latency, lsFU->isStalled);

    for (int i = 0; i < lsFU->latency; i++) {
        printf("\tstage: %i, ", i);
       
        LSFUResult *stageElement = lsFU->stages[i];
       
        if (!stageElement) {
            printf("NULL\n");
        } else {
            printf("entry: %p, base: %i, offset: %i, resultAddr: %i, destROB: %i, fuType: %s\n", 
                stageElement, stageElement->base, stageElement->offset, stageElement->resultAddr, stageElement->destROB, fuTypeToString(stageElement->fuType));
        }
    }
}

// removes the content of the the int functional unit
void flushLSFunctionalUnit(LSFunctionalUnit *lsFU) {
    
    for (int i = 0; i < lsFU->latency; i++) {
        lsFU->stages[i] = NULL;
    }

    lsFU->isStalled = 0;
}

// perform load/store functional unit operations over a cycle
void cycleLSFunctionalUnit(LSFunctionalUnit *lsFU, StatusTables *statusTables) {

    printf_DEBUG(("\nperforming load/store functional unit operations...\n"));
    
    if (lsFU->isStalled) {
        printf_DEBUG(("\tload/store functional unit is stalled because its result was not taken by the memory unit\n"));
        return;
    }

    ResStationStatusTable *resStationTable = statusTables->resStationTable;
    ROBStatusTable *robTable = statusTables->robTable;

    // get load and store reservation stations
    ResStationStatusTableEntry **loadResStationEntries = resStationEntriesForFunctionalUnit(resStationTable, FU_TYPE_LOAD);
    ResStationStatusTableEntry **storeResStationEntries = resStationEntriesForFunctionalUnit(resStationTable, FU_TYPE_STORE);
    int numLoadEntries = numResStationsForFunctionalUnit(resStationTable, FU_TYPE_LOAD);
    int numStoreEntries = numResStationsForFunctionalUnit(resStationTable, FU_TYPE_STORE);
    
    enum FunctionalUnitType closestToHeadType = FU_TYPE_NONE;
    int closestToHeadMinVal = robTable->NR;
    int closestToHeadResStationIndex = -1;

    // iterate over load reservation stations
    for (int i = 0; i < numLoadEntries; i++) {
        
        ResStationStatusTableEntry *resStationEntry = loadResStationEntries[i];
        int destROB = resStationEntry->dest;
        ROBStatusTableEntry *robEntry = robTable->entries[destROB];

        // do not allow instructions that just received a value from the CDB to execute in the same cycle
        if (resStationEntry->busy && resStationEntry->justGotOperandFromCDB) {
            resStationEntry->justGotOperandFromCDB = 0;
            continue;
        }

        // if both operands of the reservation station and the instruction's state is "issued" then it can possibly be brought into the functional unit
        if (resStationEntry->busy && (resStationEntry->vkIsAvailable) && robEntry->state == INST_STATE_ISSUED) {

            // check if it is the closest load/store instruction to the head of the rob
            int distToHead = indexDistanceToROBHead(robTable, robEntry->index);

            if (distToHead < closestToHeadMinVal) {
                closestToHeadMinVal = distToHead;
                closestToHeadResStationIndex = i;
                closestToHeadType = FU_TYPE_LOAD;
            }
        }
    }

    // iterate over store reservation stations
    for (int i = 0; i < numStoreEntries; i++) {
        
        ResStationStatusTableEntry *resStationEntry = storeResStationEntries[i];
        int destROB = resStationEntry->dest;
        ROBStatusTableEntry *robEntry = robTable->entries[destROB];

        // do not allow instructions that just received a value from the CDB to execute in the same cycle
        if (resStationEntry->busy && resStationEntry->justGotOperandFromCDB) {
            resStationEntry->justGotOperandFromCDB = 0;
            continue;
        }

        // if both operands of the reservation station and the instruction's state is "issued" then it can possibly be brought into the functional unit
        if (resStationEntry->busy && (resStationEntry->vjIsAvailable && resStationEntry->vkIsAvailable) && robEntry->state == INST_STATE_ISSUED) {

            // check if it is the closest load/store instruction to the head of the rob
            int distToHead = indexDistanceToROBHead(robTable, robEntry->index);

            if (distToHead < closestToHeadMinVal) {
                closestToHeadMinVal = distToHead;
                closestToHeadResStationIndex = i;
                closestToHeadType = FU_TYPE_STORE;
            }
        }
    }

    // reservation station entry that will be brought into the functional unit
    ResStationStatusTableEntry *resStationEntry = NULL;
    
    // check if a reservation station was found to start executing
    if (closestToHeadType != FU_TYPE_NONE) {

        if (closestToHeadType == FU_TYPE_LOAD) {
            resStationEntry = loadResStationEntries[closestToHeadResStationIndex];
        } else if (closestToHeadType == FU_TYPE_STORE) {
            resStationEntry = storeResStationEntries[closestToHeadResStationIndex];
        } else {
            printf("error: invalid type for finding closest res station to ROB head in LS functional unit, this should never happen\n");
            exit(1);
        }
    } else {
        printf_DEBUG(("no reservation station entries found for LOAD/STORE functional unit\n"));

        lsFU->stages[0] = NULL;
        return;
    }

    // ROB entry associated with the found reservation station
    ROBStatusTableEntry *robEntry = robTable->entries[resStationEntry->dest];
    robEntry->state = INST_STATE_EXECUTING;

    // perform the address calculation
    LSFUResult *nextResult = malloc(sizeof(LSFUResult));
    nextResult->base = resStationEntry->vkInt;
    nextResult->offset = resStationEntry->addr;
    nextResult->resultAddr = nextResult->base + nextResult->offset;
    nextResult->destROB = resStationEntry->dest;
    nextResult->fuType = closestToHeadType;

    // move data through the stages of the functional unit by shifting elements of the stages array to the right
    // this does not do anything given the project design as the stages array is only one element, so it's commented out, but it's good to be general
    // for (int i = lsFU->latency - 1; i >= 1; i--) {
    //     lsFU->stages[i] = lsFU->stages[i - 1];
    // }

    // move the next result into the first stage element
    lsFU->stages[0] = nextResult;
}