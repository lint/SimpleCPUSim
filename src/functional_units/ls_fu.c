
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../status_tables/status_tables.h"
#include "../misc/misc.h"
#include "ls_fu.h"

// initialize a INT functional unit struct
void initLSFunctionalUnit(LSFunctionalUnit *lsFU, int latency) {

    lsFU->latency = latency;
    lsFU->lastSelectedResStation = -1;
    lsFU->didLastSelectLoad = 0;
    lsFU->stages = malloc(lsFU->latency * sizeof(LSFUResult *)); 
    lsFU->stages[0] = NULL;
    lsFU->isStalled = 0;
    // lsFU->fuType = FU_TYPE_; // should i combine loads and stores into one???

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
    printf("int functional unit: latency: %i lastSelectedResStation: %i isStalled: %i\n", lsFU->latency, lsFU->lastSelectedResStation, lsFU->isStalled);

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

// tries to read the load buffer to bring a new entry into the functional unit, returns 1 if successful, 0 if not
int readLoadReservationStations(LSFunctionalUnit *lsFU, StatusTables *statusTables) {

    ResStationStatusTable *resStationTable = statusTables->resStationTable;
    ROBStatusTable *robTable = statusTables->robTable;

    ResStationStatusTableEntry **resStationEntries = resStationEntriesForFunctionalUnit(resStationTable, FU_TYPE_LOAD);
    int numResStations = numResStationsForFunctionalUnit(resStationTable, FU_TYPE_LOAD);
    int nextResStation = (lsFU->lastSelectedResStation + 1) % numResStations;
    LSFUResult *nextResult = NULL;

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

            printf("selecting reservation station: INT[%d] for execution\n", resStationEntry->resStationIndex);
            
            lsFU->lastSelectedResStation = nextResStation;

            // update entry in ROB to "executing"
            robEntry->state = INST_STATE_EXECUTING;

            // allocate and initialize the next result which will get passed through the stages of the functional unit
            nextResult = malloc(sizeof(LSFUResult));
            nextResult->source1 = resStationEntry->vjInt;
            nextResult->source2 = resStationEntry->vkInt;
            nextResult->destROB = destROB;

            // perform the calculation for different possible operations
            if (resStationEntry->op == FU_OP_ADD) {
                nextResult->result = nextResult->source1 + nextResult->source2;
            } else if (resStationEntry->op == FU_OP_SUB) {
                nextResult->result = nextResult->source1 - nextResult->source2;
            } else if (resStationEntry->op == FU_OP_SLT) {
                nextResult->result = nextResult->source1 < nextResult->source2;
            } else {
                printf("error: tried to start executing an instruction in the INT functional unit with an invalid operation\n");
                exit(1);
            }

            break;
        }
        
        nextResStation = (nextResStation + 1) % numResStations;
    }


}

// tries to read the store buffer to bring a new entry into the functional unit, returns 1 if successful, 0 if not
int readStoreReservationStations(LSFunctionalUnit *lsFU, StatusTables *statusTables) {

} 

// perform INT functional unit operations during a cycle
void cycleLSFunctionalUnit(LSFunctionalUnit *lsFU, StatusTables *statusTables) {
    printf("\nperforming int functional unit operations...\n");

    if (lsFU->isStalled) {
        printf("\n load/store functional unit is stalled\n");
        return;
    }

    // need to read both load and store queues

    ResStationStatusTable *resStationTable = statusTables->resStationTable;
    ROBStatusTable *robTable = statusTables->robTable;

    ResStationStatusTableEntry **resStationEntries = resStationEntriesForFunctionalUnit(resStationTable, lsFU->fuType);
    int numResStations = numResStationsForFunctionalUnit(resStationTable, lsFU->fuType);
    int nextResStation = (lsFU->lastSelectedResStation + 1) % numResStations;
    LSFUResult *nextResult = NULL;

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

            printf("selecting reservation station: INT[%d] for execution\n", resStationEntry->resStationIndex);
            
            lsFU->lastSelectedResStation = nextResStation;

            // update entry in ROB to "executing"
            robEntry->state = INST_STATE_EXECUTING;

            // allocate and initialize the next result which will get passed through the stages of the functional unit
            nextResult = malloc(sizeof(LSFUResult));
            nextResult->source1 = resStationEntry->vjInt;
            nextResult->source2 = resStationEntry->vkInt;
            nextResult->destROB = destROB;

            // perform the calculation for different possible operations
            if (resStationEntry->op == FU_OP_ADD) {
                nextResult->result = nextResult->source1 + nextResult->source2;
            } else if (resStationEntry->op == FU_OP_SUB) {
                nextResult->result = nextResult->source1 - nextResult->source2;
            } else if (resStationEntry->op == FU_OP_SLT) {
                nextResult->result = nextResult->source1 < nextResult->source2;
            } else {
                printf("error: tried to start executing an instruction in the INT functional unit with an invalid operation\n");
                exit(1);
            }

            break;
        }
        
        nextResStation = (nextResStation + 1) % numResStations;
    }

    if (!nextResult) {
        printf("no reservation station found to start executing\n");
    }

    // move data through the stages of the functional unit by shifting elements of the stages array to the right
    // this does not do anything given the project design as the stages array is only one element, so it's commented out, but it's good to be general
    // for (int i = lsFU->latency - 1; i >= 1; i--) {
    //     lsFU->stages[i] = lsFU->stages[i - 1];
    // }

    // move the next result into the first stage element
    lsFU->stages[0] = nextResult;
}