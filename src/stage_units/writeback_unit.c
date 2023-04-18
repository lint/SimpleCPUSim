
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../types/types.h"
#include "../status_tables/status_tables.h"
#include "../functional_units/functional_units.h"
#include "writeback_unit.h"

// initialize a writeback unit struct
void initWritebackUnit(WritebackUnit *writebackUnit, int NB, int NR) {
    
    writebackUnit->NB = NB;
    writebackUnit->NR = NR;
    writebackUnit->cdbsUsed = 0;

    // initialize CDBs
    writebackUnit->cdbs = malloc(NB * sizeof(CDB *));
    for (int i = 0; i < NB; i++) {
        CDB *cdb = malloc(sizeof(CDB));
        cdb->intVal = 0;
        cdb->floatVal = 0;
        cdb->robIndex = -1;
        cdb->containsData = 0;

        writebackUnit->cdbs[i] = cdb;
    }

    // initialize ROBWBInfo structs
    writebackUnit->robWBInfoArr = malloc(NR * sizeof(ROBWBInfo *));
    for (int i = 0; i < NR; i++) {
        ROBWBInfo *info = malloc(sizeof(ROBWBInfo));
        info->floatVal = 0;
        info->intVal = 0;
        info->numOperandsWaiting = 0;
        info->producedBy = VAL_FROM_NONE;
        info->producingFUType = FUType_NONE;
        info->robIndex = i;
        info->valueType = INST_VAL_NONE;

        writebackUnit->robWBInfoArr[i] = info;
    }

}

// free any elements of the writeback unit that were stored on the heap
void teardownWritebackUnit(WritebackUnit *writebackUnit) {
    
    if (writebackUnit->cdbs) {
        for (int i = 0; i < writebackUnit->NB; i++) {
            if (writebackUnit->cdbs[i]) {
                free(writebackUnit->cdbs[i]);
            }
        }
        free(writebackUnit->cdbs);
    }

    if (writebackUnit->robWBInfoArr) {
        for (int i = 0; i < writebackUnit->NR; i++) {
            if (writebackUnit->robWBInfoArr[i]) {
                free(writebackUnit->robWBInfoArr[i]);
            }
        }
        free(writebackUnit->robWBInfoArr);
    }
}

// resets the array of ROBWBInfo structs
void resetWritebackUnitROBWBInfo(WritebackUnit *writebackUnit) {

    for (int i = 0; i < writebackUnit->NR; i++) {
        ROBWBInfo *info = writebackUnit->robWBInfoArr[i];
        info->floatVal = 0;
        info->intVal = 0;
        info->numOperandsWaiting = 0;
        info->producedBy = VAL_FROM_NONE;
        info->producingFUType = FUType_NONE;
        info->valueType = INST_VAL_NONE;
        info->robIndex = i;
    }
}

// updates the number of operands in reservation station entries that are waiting for different ROBs
void updateWritebackUnitWaitingForROB(WritebackUnit *writebackUnit, ResStationStatusTableEntry **resStationEntries, int numEntries) {
    
    // iterate over given reservation stations
    for (int i = 0; i < numEntries; i++) {
        ResStationStatusTableEntry *entry = resStationEntries[i];

        // increase the counter for the source ROB if the operand is not available
        if (entry->busy) {
            if (!entry->vjIsAvailable) {
                writebackUnit->robWBInfoArr[entry->qj]->numOperandsWaiting++;
            }
            if (!entry->vkIsAvailable) {
                writebackUnit->robWBInfoArr[entry->qk]->numOperandsWaiting++;
            }
        }
    }
}

// helper method to print the contents of ROBWBInfo's 
void printWritebackUnitROBInfo(WritebackUnit *writebackUnit) {

    printf("writebackUnit ROB info arr:\n");
    for (int i = 0; i < writebackUnit->NR; i++) {
        ROBWBInfo *info = writebackUnit->robWBInfoArr[i];

        printf("\tROB: %i, numOpsWaiting: %i, intVal: %i, floatVal: %f, valueType: %s, producedByType: %s, producedByFU: %s\n", 
            info->robIndex, info->numOperandsWaiting, info->intVal, info->floatVal, 
            instResultTypeToString(info->valueType), valProducedByToString(info->producedBy), fuTypeToString(info->producingFUType));
    }
}

// helper method to print the contents of the CDBs
void printWritebackUnitCDBS(WritebackUnit *writebackUnit) {
    printf("CDBs: numUsed: %i\n", writebackUnit->cdbsUsed);
    for (int i = 0; i < writebackUnit->NB; i++) {
        CDB *cdb = writebackUnit->cdbs[i];

        printf("\tCDB: %i, containsData: %i, robIndex: %i, intVal: %i, floatVal: %f\n", i, cdb->containsData, cdb->robIndex, cdb->intVal, cdb->floatVal);
    }
}


// perform writeback unit operations during a cycle
void cycleWritebackUnit(WritebackUnit *writebackUnit, StatusTables *statusTables, FunctionalUnits *functionalUnits) {

    printf("\nperforming writeback unit operations...\n");

    // clear current CDB values
    for (int i = 0; i < writebackUnit->NB; i++) {
        CDB *cdb = writebackUnit->cdbs[i];
        cdb->intVal = 0;
        cdb->floatVal = 0;
        cdb->robIndex = -1;
        cdb->containsData = 0;
    }
    writebackUnit->cdbsUsed = 0;


    ROBStatusTable *robTable = statusTables->robTable;
    ResStationStatusTable *resStationTable = statusTables->resStationTable;
    
    resetWritebackUnitROBWBInfo(writebackUnit);

    // check if ROB status table contains value for each ROB entry
    for (int i = 0; i < writebackUnit->NR; i++) {

        ROBStatusTableEntry *robEntry = robTable->entries[i];

        if (robEntry->state == STATE_WROTE_RESULT) {
            ROBWBInfo *info = writebackUnit->robWBInfoArr[i];

            info->producedBy = VAL_FROM_ROB;

            if (robEntry->instResultValueType == INST_VAL_INT) {
                info->intVal = robEntry->intValue;
            } else if (robEntry->instResultValueType == INST_VAL_FLOAT) {
                info->floatVal = robEntry->floatValue;
            }
        }
    }

    // check the produced results from each functional unit
    IntFUResult *currIntFUResult = getCurrentIntFunctionalUnitResult(functionalUnits->intFU);
    if (currIntFUResult) {
        printf("destROB: %i, value: %i read from INT functional unit result\n", currIntFUResult->destROB, currIntFUResult->result);

        ROBWBInfo *info = writebackUnit->robWBInfoArr[currIntFUResult->destROB];

        if (info->producedBy != VAL_FROM_NONE) {
            printf("error: destination ROB found in functional unit and wrote result to ROB status table, this shouldn't happen...\n");
            // TOOD: return or exit?
        }

        info->producedBy = VAL_FROM_FU;
        info->producingFUType = FUType_INT;
        info->intVal = currIntFUResult->result;
        info->valueType = INST_VAL_INT;

        // stall functional unit by default and remove the stall later if its result is placed on the CDB
        functionalUnits->intFU->isStalled = 1;
    } else {
        printf("no result available from INT functional unit\n");
    }


    // TODO: need to add every other functional unit here as well

    // increase the robsNeededBy counters for every functional unit's reservation stations
    updateWritebackUnitWaitingForROB(writebackUnit, resStationTable->intEntries, resStationTable->numIntStations);
    updateWritebackUnitWaitingForROB(writebackUnit, resStationTable->loadEntries, resStationTable->numLoadStations);
    updateWritebackUnitWaitingForROB(writebackUnit, resStationTable->storeEntries, resStationTable->numStoreStations);
    updateWritebackUnitWaitingForROB(writebackUnit, resStationTable->fpAddEntries, resStationTable->numFPAddStations);
    updateWritebackUnitWaitingForROB(writebackUnit, resStationTable->fpMultEntries, resStationTable->numFPMultStations);
    updateWritebackUnitWaitingForROB(writebackUnit, resStationTable->fpDivEntries, resStationTable->numFPDivStations);
    updateWritebackUnitWaitingForROB(writebackUnit, resStationTable->buEntries, resStationTable->numBUStations);

    printWritebackUnitROBInfo(writebackUnit);

    // find the ROB values to send on the CDB
    int robValsToSend[writebackUnit->NB];
    for (int i = 0; i < writebackUnit->NB; i++) {

        // find the ROB dest with the maximum number of operands waiting on it
        int maxOpsWaiting = 0;
        int maxOpsROB = -1;

        // iterate over every ROBWBInfo struct
        for (int j = 0; j < writebackUnit->NR; j++) {

            // check if the current ROB dest is already in the CDB
            int alreadyInCDB = 0;
            for (int k = 0; k < writebackUnit->cdbsUsed; k++) {
                if (writebackUnit->cdbs[k]->robIndex == j) {
                    alreadyInCDB = 1;
                    break;
                }
            }
            if (alreadyInCDB) {
                continue;
            }

            // check the ROBWBInfo struct for the number of operands waiting for it
            ROBWBInfo *info = writebackUnit->robWBInfoArr[j];
            if (info->producingFUType != VAL_FROM_NONE) {
                if (info->numOperandsWaiting > maxOpsWaiting) {
                    maxOpsWaiting = info->numOperandsWaiting;
                    maxOpsROB = info->robIndex; // same as j
                }
            }
        }

        if (maxOpsROB == -1) {
            printf("no ROB with maximum number of waiting operands found\n");
            break;
        } else {
            
            ROBWBInfo *info = writebackUnit->robWBInfoArr[maxOpsROB];
            CDB *cdb = writebackUnit->cdbs[writebackUnit->cdbsUsed];

            cdb->containsData = 1;
            cdb->robIndex = maxOpsROB;
            
            if (info->valueType == INST_VAL_INT) {
                cdb->intVal = info->intVal;
                printf("added robIndex: %i (operandsWaiting: %i) intVal: %i to CDB: %i\n", cdb->robIndex, info->numOperandsWaiting, cdb->intVal, writebackUnit->cdbsUsed);
            } else if (info->valueType == INST_VAL_FLOAT) {
                cdb->floatVal = info->floatVal;
                printf("added robIndex: %i (operandsWaiting: %i) floatVal: %f to CDB: %i\n", cdb->robIndex, info->numOperandsWaiting, cdb->floatVal, writebackUnit->cdbsUsed);
            } else {
                printf("error: value type not found when adding value to CDB\n");
            }

            writebackUnit->cdbsUsed++;
        }
    }

    // if CDB still has entries available, try to add more values from functional unit results
    if (writebackUnit->cdbsUsed < writebackUnit->NB) {

        // iterate over every ROBWBInfo struct
        for (int i = 0; i < writebackUnit->NR && writebackUnit->cdbsUsed < writebackUnit->NB; i++) {
            ROBWBInfo *info = writebackUnit->robWBInfoArr[i];

            // check if the current ROB dest is already in the CDB
            int alreadyInCDB = 0;
            for (int j = 0; j < writebackUnit->cdbsUsed; j++) {
                if (writebackUnit->cdbs[j]->robIndex == i) {
                    alreadyInCDB = 1;
                    break;
                }
            }
            if (alreadyInCDB) {
                continue;
            }

            if (info->producedBy == VAL_FROM_FU) {
                CDB *cdb = writebackUnit->cdbs[writebackUnit->cdbsUsed];

                cdb->robIndex = info->robIndex;
                cdb->containsData = 1;
                
                if (info->valueType == INST_VAL_INT) {
                    cdb->intVal = info->intVal;
                    printf("added robIndex: %i (extra from FU) intVal: %i to CDB: %i\n", cdb->robIndex, cdb->intVal, writebackUnit->cdbsUsed);
                } else if (info->valueType == INST_VAL_FLOAT) {
                    cdb->floatVal = info->floatVal;
                    printf("added robIndex: %i (extra from FU) floatVal: %f to CDB: %i\n", cdb->robIndex, cdb->floatVal, writebackUnit->cdbsUsed);
                } else {
                    printf("error: value type not found when adding value to CDB (extra FU addition)\n");
                }

                writebackUnit->cdbsUsed++;
            }
        }
    }

    // update ROB status table for values added to CDB and remove the stall from functional units if necessary
    for (int i = 0; i < writebackUnit->cdbsUsed; i++) {
        CDB *cdb = writebackUnit->cdbs[i];
        ROBWBInfo *info = writebackUnit->robWBInfoArr[cdb->robIndex];

        // check if the value sent on the CDB came from a functional unit
        if (info->producedBy == VAL_FROM_FU) {

            // update the ROB status table
            ROBStatusTableEntry *robStatusEntry = robTable->entries[cdb->robIndex];
            robStatusEntry->state = STATE_WROTE_RESULT;

            if (info->valueType == INST_VAL_INT) {
                robStatusEntry->intValue = cdb->intVal;
            } else {
                robStatusEntry->floatValue = cdb->floatVal;
            }

            // update reservation station status table
            ResStationStatusTableEntry *resStationEntry = resStationEntryForFunctionalUnitWithDestROB(resStationTable, info->producingFUType, cdb->robIndex);
            resStationEntry->busy = 0;

            // don't actually stall the functional unit
            if (info->producingFUType == FUType_INT) {
                functionalUnits->intFU->isStalled = 0;
            } else if (info->producingFUType == FUType_LOAD) {

            } else if (info->producingFUType == FUType_STORE) {

            } else if (info->producingFUType == FUType_FPAdd) {

            } else if (info->producingFUType == FUType_FPMult) {

            } else if (info->producingFUType == FUType_FPDiv) {

            } else if (info->producingFUType == FUType_BU) {

            } else {
                printf("error: invalid FUType when removing stall from functional unit for CDB: %i\n", i);
            }

            
        }   
    }

        // make one array with an entry for each ROB
    // iterate over every reservation station and count the number of operands waiting for each ROB
    // find the ROB index which is needed by the most operands 
        // if tie, break it by choosing the ROB which is closest to the head
        // if operand comes from a functional unit, need to update the ROB status table with the value and set the state to "wrote result"
        // prioritize writeback unit over ROB for supplying values then? so that stalls are less likely to happen?
        // if space remaining on CDB after supplying needed operands, just take values from some functional unit (that was not already chosen)
            // and if there is still space remaining, supply values from ROB closest to head?

    // also need to free up the reservation station if putting value from functional unit on the CDB
    // again, does this affect if another instruction would enter the reservation station in the same cycle when it's not supposed to?
    
    // can i issue an instruction to a reservation station and get the value from the CDB in the same cycle? why not.
    // TODO: remove this
}