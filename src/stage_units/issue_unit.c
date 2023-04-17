
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "issue_unit.h"
#include "../types/types.h"
#include "../status_tables/status_tables.h"
#include "../cpu.h"

// initialize a issue unit struct
void initIssueUnit(IssueUnit *issueUnit, int NW, int NI, Instruction **instDecodeQueue, int *numInstsInQueue) {

    issueUnit->NW = NW;
    issueUnit->NI = NI;

    issueUnit->instDecodeQueue = instDecodeQueue;
    issueUnit->numInstsInQueue = numInstsInQueue;
    // issueUnit->inputQueue = calloc(NI, sizeof(Instruction *));
    // issueUnit->numInstsInQueue = 0;
    // issueUnit->numInstsIssued = 0;
}

// free any data elements of the issue unit that are stored on the heap
void teardownIssueUnit(IssueUnit *issueUnit) {

    // if (issueUnit->inputQueue) {
    //     free(issueUnit->inputQueue);
    // }
}

// execute issue unit's operations during a cycle
void cycleIssueUnit(IssueUnit *issueUnit, RegisterFile *regFile, 
    ROBStatusTable *robTable, ResStationStatusTable *resStationTable, RegisterStatusTable *regTable, 
    StallStats *stallStats) {

    printf("\nperforming issue unit operations...\n");

    int numInstsIssued = 0;

    // check if there are any instructions in the decode queue
    if (*issueUnit->numInstsInQueue == 0) {
        printf("no instructions in decode queue\n");
        return;
    }

    // attempt to issue up to NW instructions 
    for (int i = 0; i < issueUnit->NW && numInstsIssued < *issueUnit->numInstsInQueue; i++) {

        Instruction *inst = issueUnit->instDecodeQueue[i];

        // issue instruction if free slot in ROB and reservation station is available
        if (isFreeEntryInROB(robTable) && isFreeResStationForInstruction(resStationTable, inst)) {
            numInstsIssued++;
            int robIndex = addInstToROB(robTable, inst);
            addInstToResStation(resStationTable, regTable, regFile, inst, robIndex);
            setRegisterStatusTableEntryVal(regTable, inst->destPhysReg, robIndex);
        } else {

            // check if failure to issue was caused by the ROB being full
            if (!isFreeEntryInROB(robTable)) {
                stallStats->fullROBStalls++;
                printf("encountered stall in issue unit due to ROB being full, fullROBStalls: %i\n", stallStats->fullROBStalls);
            
            // check if the failure to issue was caused by no reservation stations being available
            } else if (!isFreeResStationForInstruction(resStationTable, inst)) {
                stallStats->fullResStationStalls++;
                printf("encountered stall in issue unit due to reservation stations needed by the instruction being full, fullResStationStalls: %i\n", stallStats->fullResStationStalls);
            }

            // note: 

            break;
        }
    }

    // remove issued instructions from the decode queue
    if (numInstsIssued > 0) {
        printf("removing instructions from decode queue\n");

        for (int i = numInstsIssued; i < *issueUnit->numInstsInQueue; i++) {
            issueUnit->instDecodeQueue[i - numInstsIssued] = issueUnit->instDecodeQueue[i];
            issueUnit->instDecodeQueue[i] = NULL;
        }

        *issueUnit->numInstsInQueue -= numInstsIssued;
    }
}