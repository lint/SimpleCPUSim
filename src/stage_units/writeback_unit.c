
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../misc/misc.h"
#include "../status_tables/status_tables.h"
#include "../functional_units/functional_units.h"
#include "../memory/memory.h"
#include "writeback_unit.h"
#include "decode_unit.h"

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
        cdb->valueType = VALUE_TYPE_NONE;
        cdb->robIndex = -1;
        cdb->destPhysReg = PHYS_REG_NONE;
        cdb->producedBy = VALUE_FROM_NONE;
        cdb->producingFUType = FU_TYPE_NONE;

        writebackUnit->cdbs[i] = cdb;
    }

    // initialize ROBWBInfo structs
    writebackUnit->robWBInfoArr = malloc(NR * sizeof(ROBWBInfo *));
    for (int i = 0; i < NR; i++) {
        ROBWBInfo *info = malloc(sizeof(ROBWBInfo));
        info->floatVal = 0;
        info->intVal = 0;
        info->numOperandsWaiting = 0;
        info->producedBy = VALUE_FROM_NONE;
        info->producingFUType = FU_TYPE_NONE;
        info->robIndex = i;
        info->valueType = VALUE_TYPE_NONE;

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
        info->producedBy = VALUE_FROM_NONE;
        info->producingFUType = FU_TYPE_NONE;
        info->valueType = VALUE_TYPE_NONE;
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
            valueTypeToString(info->valueType), valueProducedByToString(info->producedBy), fuTypeToString(info->producingFUType));
    }
}

// helper method to print the contents of the CDBs
void printWritebackUnitCDBS(WritebackUnit *writebackUnit) {
    printf("CDBs: numUsed: %i\n", writebackUnit->cdbsUsed);
    for (int i = 0; i < writebackUnit->NB; i++) {
        CDB *cdb = writebackUnit->cdbs[i];

        printf("\tCDB: %i, robIndex: %i, intVal: %i, floatVal: %f, valueType: %s, destPhysReg: %s, producedByType: %s producingFUType: %s\n", i, cdb->robIndex, cdb->intVal, cdb->floatVal, 
            valueTypeToString(cdb->valueType), physicalRegisterNameToString(cdb->destPhysReg), valueProducedByToString(cdb->producedBy), fuTypeToString(cdb->producingFUType));
    }
}

// adds a ROB entry to the CDB
void addROBEntryToCDB(WritebackUnit *writebackUnit, ROBStatusTableEntry *entry) {
    CDB *cdb = writebackUnit->cdbs[writebackUnit->cdbsUsed++];
    cdb->producedBy = VALUE_FROM_ROB;
    cdb->destPhysReg = entry->renamedDestReg;
    cdb->robIndex = entry->index;
    cdb->valueType = entry->instResultValueType;
    
    if (cdb->valueType == VALUE_TYPE_INT) {
        cdb->intVal = entry->intValue;
    } else if (cdb->valueType == VALUE_TYPE_FLOAT) {
        cdb->floatVal = entry->floatValue;
    } else {
        printf("error: ROB: %i to commit has an invalid value type, this shouldn't happen\n", entry->index);
    }
}

// perform writeback unit operations during a cycle
void cycleWritebackUnit(WritebackUnit *writebackUnit, DecodeUnit *decodeUnit, StatusTables *statusTables, FunctionalUnits *functionalUnits, RegisterFile *registerFile) {

    printf("\nperforming writeback unit operations...\n");

    ROBStatusTable *robTable = statusTables->robTable;
    ResStationStatusTable *resStationTable = statusTables->resStationTable;
    RegisterStatusTable *regTable = statusTables->regTable;

    // clear current CDB values
    for (int i = 0; i < writebackUnit->NB; i++) {
        CDB *cdb = writebackUnit->cdbs[i];
        cdb->intVal = 0;
        cdb->floatVal = 0;
        cdb->valueType = VALUE_TYPE_NONE;
        cdb->robIndex = -1;
        cdb->destPhysReg = PHYS_REG_NONE;
        cdb->producedBy = VALUE_FROM_NONE;
        cdb->producingFUType = FU_TYPE_NONE;
    }
    writebackUnit->cdbsUsed = 0;

    resetWritebackUnitROBWBInfo(writebackUnit);

    // check and see if the head of the ROB is able to be added to the CDB to be committed
    int numInstsToCommit = 0;
    ROBStatusTableEntry *headROBEntry = getHeadROBEntry(robTable);
    if (headROBEntry->state == STATE_WROTE_RESULT) {
        printf("head of ROB (%i) is ready to commit, adding it to the CDB\n", headROBEntry->index);

        addROBEntryToCDB(writebackUnit, headROBEntry);
        numInstsToCommit++;
    }


    // check the produced results from INT functional unit
    IntFUResult *currIntFUResult = getCurrentIntFunctionalUnitResult(functionalUnits->intFU);
    if (currIntFUResult) {
        printf("destROB: %i, value: %i read from INT functional unit result\n", currIntFUResult->destROB, currIntFUResult->result);

        ROBWBInfo *info = writebackUnit->robWBInfoArr[currIntFUResult->destROB];

        if (info->producedBy != VALUE_FROM_NONE) {
            printf("error: destination ROB found in functional unit was already seen, this shouldn't happen...\n");
            // TOOD: return or exit?
        }

        info->producedBy = VALUE_FROM_FU;
        info->producingFUType = FU_TYPE_INT;
        info->intVal = currIntFUResult->result;
        info->valueType = VALUE_TYPE_INT;

        // stall functional unit by default and remove the stall later if its result is placed on the CDB
        functionalUnits->intFU->isStalled = 1;
    } else {
        printf("no result available from INT functional unit\n");
    }

    // check the produced results from FPAdd functional unit
    FloatFUResult *currFPAddFUResult = getCurrentFPFunctionalUnitResult(functionalUnits->fpAddFU);
    if (currFPAddFUResult) {
        printf("destROB: %i, value: %f read from FPAdd functional unit result\n", currFPAddFUResult->destROB, currFPAddFUResult->result);

        ROBWBInfo *info = writebackUnit->robWBInfoArr[currFPAddFUResult->destROB];

        if (info->producedBy != VALUE_FROM_NONE) {
            printf("error: destination ROB found in functional unit was already seen, this shouldn't happen...\n");
            // TOOD: return or exit?
        }

        info->producedBy = VALUE_FROM_FU;
        info->producingFUType = FU_TYPE_FPADD;
        info->floatVal = currFPAddFUResult->result;
        info->valueType = VALUE_TYPE_FLOAT;

        // stall functional unit by default and remove the stall later if its result is placed on the CDB
        functionalUnits->fpAddFU->isStalled = 1;
    } else {
        printf("no result available from FPAdd functional unit\n");
    }

    // check the produced results from FPMul functional unit
    FloatFUResult *currFpMulFUResult = getCurrentFPFunctionalUnitResult(functionalUnits->fpMulFU);
    if (currFpMulFUResult) {
        printf("destROB: %i, value: %f read from FPMul functional unit result\n", currFpMulFUResult->destROB, currFpMulFUResult->result);

        ROBWBInfo *info = writebackUnit->robWBInfoArr[currFpMulFUResult->destROB];

        if (info->producedBy != VALUE_FROM_NONE) {
            printf("error: destination ROB found in functional unit was already seen, this shouldn't happen...\n");
            // TOOD: return or exit?
        }

        info->producedBy = VALUE_FROM_FU;
        info->producingFUType = FU_TYPE_FPMUL;
        info->floatVal = currFpMulFUResult->result;
        info->valueType = VALUE_TYPE_FLOAT;

        // stall functional unit by default and remove the stall later if its result is placed on the CDB
        functionalUnits->fpMulFU->isStalled = 1;
    } else {
        printf("no result available from FPMul functional unit\n");
    }

    // check the produced results from FPMul functional unit
    FloatFUResult *currFpDivFUResult = getCurrentFPFunctionalUnitResult(functionalUnits->fpDivFU);
    if (currFpDivFUResult) {
        printf("destROB: %i, value: %f read from FPDiv functional unit result\n", currFpDivFUResult->destROB, currFpDivFUResult->result);

        ROBWBInfo *info = writebackUnit->robWBInfoArr[currFpDivFUResult->destROB];

        if (info->producedBy != VALUE_FROM_NONE) {
            printf("error: destination ROB found in functional unit was already seen, this shouldn't happen...\n");
            // TOOD: return or exit?
        }

        info->producedBy = VALUE_FROM_FU;
        info->producingFUType = FU_TYPE_FPDIV;
        info->floatVal = currFpDivFUResult->result;
        info->valueType = VALUE_TYPE_FLOAT;

        // stall functional unit by default and remove the stall later if its result is placed on the CDB
        functionalUnits->fpDivFU->isStalled = 1;
    } else {
        printf("no result available from FPDiv functional unit\n");
    }

    // TODO: need to add every other functional unit here as well

    // increase the robsNeededBy counters for every functional unit's reservation stations
    updateWritebackUnitWaitingForROB(writebackUnit, resStationTable->intEntries, resStationTable->numIntStations);
    updateWritebackUnitWaitingForROB(writebackUnit, resStationTable->loadEntries, resStationTable->numLoadStations);
    updateWritebackUnitWaitingForROB(writebackUnit, resStationTable->storeEntries, resStationTable->numStoreStations);
    updateWritebackUnitWaitingForROB(writebackUnit, resStationTable->fpAddEntries, resStationTable->numFPAddStations);
    updateWritebackUnitWaitingForROB(writebackUnit, resStationTable->fpMulEntries, resStationTable->numFPMulStations);
    updateWritebackUnitWaitingForROB(writebackUnit, resStationTable->fpDivEntries, resStationTable->numFPDivStations);
    updateWritebackUnitWaitingForROB(writebackUnit, resStationTable->buEntries, resStationTable->numBUStations);

    printWritebackUnitROBInfo(writebackUnit);

    // find which values from functional units to send on the CDB
    for (int i = 0; i < writebackUnit->NB && writebackUnit->cdbsUsed < writebackUnit->NB; i++) {

        // find the ROB dest with the maximum number of operands waiting on it
        int maxOpsWaiting = -1;
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
            if (info->producedBy == VALUE_FROM_FU) {
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

            cdb->producedBy = VALUE_FROM_FU;
            cdb->robIndex = maxOpsROB;
            cdb->producingFUType = info->producingFUType;
            
            if (info->valueType == VALUE_TYPE_INT) {
                cdb->intVal = info->intVal;
                cdb->valueType = VALUE_TYPE_INT;
                printf("added robIndex: %i (operandsWaiting: %i) intVal: %i to CDB: %i\n", cdb->robIndex, info->numOperandsWaiting, cdb->intVal, writebackUnit->cdbsUsed);
            } else if (info->valueType == VALUE_TYPE_FLOAT) {
                cdb->floatVal = info->floatVal;
                cdb->valueType = VALUE_TYPE_FLOAT;
                printf("added robIndex: %i (operandsWaiting: %i) floatVal: %f to CDB: %i\n", cdb->robIndex, info->numOperandsWaiting, cdb->floatVal, writebackUnit->cdbsUsed);
            } else {
                printf("error: value type not found when adding value to CDB\n");
            }

            writebackUnit->cdbsUsed++;
        }
    }

    // TODO: remove this
    // if CDB still has entries available, try to add more values from functional unit results
    // if (writebackUnit->cdbsUsed < writebackUnit->NB) {

    //     // iterate over every ROBWBInfo struct
    //     for (int i = 0; i < writebackUnit->NR && writebackUnit->cdbsUsed < writebackUnit->NB; i++) {
    //         ROBWBInfo *info = writebackUnit->robWBInfoArr[i];

    //         // check if the current ROB dest is already in the CDB
    //         int alreadyInCDB = 0;
    //         for (int j = 0; j < writebackUnit->cdbsUsed; j++) {
    //             if (writebackUnit->cdbs[j]->robIndex == i) {
    //                 alreadyInCDB = 1;
    //                 break;
    //             }
    //         }
    //         if (alreadyInCDB) {
    //             continue;
    //         }

    //         if (info->producedBy == VALUE_FROM_FU) {
    //             CDB *cdb = writebackUnit->cdbs[writebackUnit->cdbsUsed];

    //             cdb->robIndex = info->robIndex;
    //             cdb->producedByType = VALUE_FROM_FU;
                
    //             if (info->valueType == VALUE_TYPE_INT) {
    //                 cdb->intVal = info->intVal;
    //                 cdb->valueType = VALUE_TYPE_INT;
    //                 printf("added robIndex: %i (extra from FU) intVal: %i to CDB: %i\n", cdb->robIndex, cdb->intVal, writebackUnit->cdbsUsed);
    //             } else if (info->valueType == VALUE_TYPE_FLOAT) {
    //                 cdb->floatVal = info->floatVal;
    //                 cdb->valueType = VALUE_TYPE_FLOAT;
    //                 printf("added robIndex: %i (extra from FU) floatVal: %f to CDB: %i\n", cdb->robIndex, cdb->floatVal, writebackUnit->cdbsUsed);
    //             } else {
    //                 printf("error: value type not found when adding value to CDB (extra FU addition)\n");
    //             }

    //             writebackUnit->cdbsUsed++;
    //         }
    //     }
    // }

    // if CDB still has space available, try to add more instructions to commit
    if (writebackUnit->cdbsUsed < writebackUnit->NB) {

        printf("space remaining in CDB, attempting to add more instructions to commit\n");

        for (int i = 0; i < writebackUnit->NB - writebackUnit->cdbsUsed; i++) {
            ROBStatusTableEntry *nextHeadEntry = robTable->entries[robTable->headEntryIndex + numInstsToCommit];

            if (nextHeadEntry->state == STATE_WROTE_RESULT) {
                printf("head of ROB (%i) is ready to commit, adding it to the CDB\n", nextHeadEntry->index);

                addROBEntryToCDB(writebackUnit, nextHeadEntry);
                numInstsToCommit++;
            } else {
                printf("ROB: %i not ready to commit\n", nextHeadEntry->index);
            }
        }
    }

    /* commit stage */

    // iterate over every entry in the CDB
    for (int i = 0; i < writebackUnit->cdbsUsed; i++) {
        CDB *cdb = writebackUnit->cdbs[i];

        // check if the value sent on the CDB came from a functional unit
        if (cdb->producedBy == VALUE_FROM_FU) {

            printf("CDB: %i contains value from a functional unit, updating ROB: %i to WROTE_RESULT and associated reservation station to busy = 0\n", i, cdb->robIndex);

            // update the ROB status table and reservation stations that are waiting for the result
            ROBStatusTableEntry *robStatusEntry = robTable->entries[cdb->robIndex];
            robStatusEntry->state = STATE_WROTE_RESULT;

            if (cdb->valueType == VALUE_TYPE_INT) {
                robStatusEntry->intValue = cdb->intVal;
                sendIntUpdateToResStationStatusTable(resStationTable, cdb->robIndex, cdb->intVal, 1);
            } else {
                robStatusEntry->floatValue = cdb->floatVal;
                sendFloatUpdateToResStationStatusTable(resStationTable, cdb->robIndex, cdb->floatVal, 1);
            }

            // update reservation station status table to free reservation station that produced the result
            ResStationStatusTableEntry *resStationEntry = resStationEntryForFunctionalUnitWithDestROB(resStationTable, cdb->producingFUType, cdb->robIndex);
            resStationEntry->busy = 0;

            // don't actually stall the functional unit
            if (cdb->producingFUType == FU_TYPE_INT) {
                functionalUnits->intFU->isStalled = 0;
            } else if (cdb->producingFUType == FU_TYPE_LOAD) {

            } else if (cdb->producingFUType == FU_TYPE_STORE) {

            } else if (cdb->producingFUType == FU_TYPE_FPADD) {
                functionalUnits->fpAddFU->isStalled = 0;
            } else if (cdb->producingFUType == FU_TYPE_FPMUL) {
                functionalUnits->fpMulFU->isStalled = 0;
            } else if (cdb->producingFUType == FU_TYPE_FPDIV) {
                functionalUnits->fpDivFU->isStalled = 0;
            } else if (cdb->producingFUType == FU_TYPE_BU) {

            } else {
                printf("error: invalid FUType when removing stall from functional unit for CDB: %i\n", i);
            }

            // TODO: again this for ever functional unit
        
        // value came from the ROB (instruction will commit)
        } else if (cdb->producedBy == VALUE_FROM_ROB) {

            printf("CDB: %i contains value from ROB, updating ROB: %i to COMMITED and writing the result to register file\n", i, cdb->robIndex);

            // update ROB status table
            ROBStatusTableEntry *robStatusEntry = robTable->entries[cdb->robIndex];
            robStatusEntry->busy = 0;
            robStatusEntry->state = STATE_COMMIT;

            // update reservation stations with value
            if (cdb->valueType == VALUE_TYPE_INT) {
                sendIntUpdateToResStationStatusTable(resStationTable, cdb->robIndex, cdb->intVal, 1);
            } else {
                sendFloatUpdateToResStationStatusTable(resStationTable, cdb->robIndex, cdb->floatVal, 1);
            }

            // update register status table
            RegisterStatusTableEntry *regStatusEntry = registerStatusTableEntryForReg(regTable, robStatusEntry->destReg);

            // reset the register status table entry if the committed instruction's destination ROB matches the current entry's ROB
            if (regStatusEntry->robIndex == robStatusEntry->index) {
                regStatusEntry->robIndex = -1;
            }

            // TODO: loads/stores/branches

            // update free list
            popOldPhysicalRegisterMappingForReg(decodeUnit, robStatusEntry->destReg);

            // update register file
            if (cdb->valueType == VALUE_TYPE_INT) {
                printf("\tcommiting: %i to register: %s\n", cdb->intVal, physicalRegisterNameToString(cdb->destPhysReg));
                writeRegisterFileInt(registerFile, cdb->destPhysReg, cdb->intVal);
            } else if (cdb->valueType == VALUE_TYPE_FLOAT) {
                printf("\tcommiting: %f to register: %s\n", cdb->floatVal, physicalRegisterNameToString(cdb->destPhysReg));
                writeRegisterFileFloat(registerFile, cdb->destPhysReg, cdb->floatVal);
            }

            robTable->headEntryIndex = (robStatusEntry->index+1) % robTable->NR;
            printf("\tupdating ROB head index to: %i\n", robTable->headEntryIndex);
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