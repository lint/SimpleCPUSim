
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "functional_unit_types.h"
#include "../status_tables/status_tables.h"
#include "../types/types.h"
#include "int_fu.h"

// initialize a INT functional unit struct
void initIntFunctionalUnit(IntFunctionalUnit *intFU) {

    intFU->latency = 1; // defined in the project description
    intFU->lastSelectedResStation = -1;
    intFU->stages = malloc(intFU->latency * sizeof(IntFUResult *)); 
    intFU->stages[0] = NULL;
    intFU->fuType = FUType_INT;
}

// free any INT functional unit elements that are stored on the heap
void teardownIntFunctionalUnit(IntFunctionalUnit *intFU) {
    if (intFU->stages) {
        free(intFU->stages);
    }
}

// gets the last element in the stages array, which indicates the most recent completed result
IntFUResult *getCurrentIntFunctionalUnitResult(IntFunctionalUnit *intFU) {
    return intFU->stages[intFU->latency-1];
}

// helper method to print the contents of the int functional unit
void printIntFunctionalUnit(IntFunctionalUnit *intFU) {
    printf("int functional unit: latency: %i lastSelectedResStation: %i\n", intFU->latency, intFU->lastSelectedResStation);

    for (int i = 0; i < intFU->latency; i++) {
        printf("\tstage: %i, ", i);
        IntFUResult *stageElement = intFU->stages[i];
        if (!stageElement) {
            printf("NULL\n");
        } else {
            printf("entry: %p, source1: %i, source2: %i, result: %i, destROB: %i\n", 
                stageElement, stageElement->source1, stageElement->source2, stageElement->result, stageElement->destROB);
        }
    }
}

// perform INT functional unit operations during a cycle
void cycleIntFunctionalUnit(IntFunctionalUnit *intFU, ResStationStatusTable *resStationTable, ROBStatusTable *robTable) {
    printf("\nperforming int functional unit operations...\n");

    ResStationStatusTableEntry **resStationEntries = resStationEntriesForFunctionalUnit(resStationTable, intFU->fuType);
    int numResStations = numResStationsForFunctionalUnit(resStationTable, intFU->fuType);
    int nextResStation = (intFU->lastSelectedResStation + 1) % numResStations;
    IntFUResult *nextResult = NULL;

    // iterate over all reservation stations to find the next operands to operate on
    for (int i = 0; i < numResStations; i++) {

        // get the reservation station entry's destination ROB and use it to get the assocaited ROB entry
        ResStationStatusTableEntry *resStationEntry = resStationTable->intEntries[nextResStation];
        int destROB = resStationEntry->dest;
        ROBStatusTableEntry *robEntry = robTable->entries[destROB];

        // if both operands of the reservation station and the instruction's state is "issued" then it can be brought into the functional unit
        if ((resStationEntry->vjIsAvailable && resStationEntry->vkIsAvailable) && robEntry->state == STATE_ISSUED) {

            printf("selecting reservation station: INT[%d] for execution\n", resStationEntry->resStationIndex);
            
            intFU->lastSelectedResStation = nextResStation;

            // update entry in ROB to "executing"
            robEntry->state = STATE_EXECUTING;

            // allocate and initialize the next result which will get passed through the stages of the functional unit
            nextResult = malloc(sizeof(IntFUResult));
            nextResult->source1 = resStationEntry->vjInt;
            nextResult->source2 = resStationEntry->vkInt;
            nextResult->destROB = destROB;

            // perform the calculation for different possible operations
            if (resStationEntry->op == FUOp_ADD) {
                nextResult->result = nextResult->source1 + nextResult->source2;
            } else if (resStationEntry->op == FUOp_SUB) {
                nextResult->result = nextResult->source1 - nextResult->source2;
            } else if (resStationEntry->op == FUOp_SLT) {
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
    // this does not do anything in the current use case as the stages array is only one element, but i wanted to make it general
    for (int i = 1; i < intFU->latency; i++) {
        intFU->stages[i] = intFU->stages[i - 1];
    }

    // move the next result into the first stage element
    intFU->stages[0] = nextResult;
}