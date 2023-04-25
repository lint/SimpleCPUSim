
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../misc/misc.h"
#include "../status_tables/status_tables.h"
#include "../functional_units/functional_units.h"
#include "../memory/memory.h"
#include "../branch_prediction/branch_predictor.h"
#include "mem_unit.h"
#include "writeback_unit.h"
#include "fetch_unit.h"
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
        info->addr = -1;

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
        info->addr = -1;
    }
}

// updates the number of operands in reservation station entries that are waiting for different ROBs
void updateWritebackUnitWaitingForROB(WritebackUnit *writebackUnit, ResStationStatusTableEntry **resStationEntries, int numEntries) {
    
    // iterate over given reservation stations
    for (int i = 0; i < numEntries; i++) {
        ResStationStatusTableEntry *entry = resStationEntries[i];

        // printf("entry: %p, \n", entry);

        // increase the counter for the source ROB if the operand is not available
        if (entry->busy) {
            if (!entry->vjIsAvailable && entry->qj != -1) {
                writebackUnit->robWBInfoArr[entry->qj]->numOperandsWaiting++;
            }
            if (!entry->vkIsAvailable && entry->qk != -1) {
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
    cdb->producingFUType = entry->fuType;

    if (entry->fuType == FU_TYPE_BU) { // TODO: don't really need the load address on the CDB right?
        cdb->addr = entry->addr;
    } else if (entry->fuType == FU_TYPE_STORE || entry->fuType == FU_TYPE_LOAD) {
        cdb->addr = entry->addr;
        cdb->floatVal = entry->floatValue;
    } else {
        if (cdb->valueType == VALUE_TYPE_INT) {
            cdb->intVal = entry->intValue;
        } else if (cdb->valueType == VALUE_TYPE_FLOAT) {
            cdb->floatVal = entry->floatValue;
        } else {
            printf("error: ROB: %i to commit has an invalid value type, this shouldn't happen\n", entry->index);
        }
    }

}

// perform writeback unit operations during a cycle
void cycleWritebackUnit(WritebackUnit *writebackUnit, FetchUnit *fetchUnit, DecodeUnit *decodeUnit, MemoryUnit *memUnit, StatusTables *statusTables, 
    FunctionalUnits *functionalUnits, RegisterFile *registerFile, DataCache *dataCache, BranchPredictor *branchPredictor) {

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

    // make sure you don't overfill cdb without checking capacity

    // check and see if the head of the ROB is able to be added to the CDB to be committed
    int numInstsToCommit = 0;
    // ROBStatusTableEntry *headROBEntry = getHeadROBEntry(robTable);
    // if (headROBEntry->busy && headROBEntry->state == INST_STATE_WROTE_RESULT) {
    //     printf("head of ROB (%i) is ready to commit, adding it to the CDB\n", headROBEntry->index);

    //     addROBEntryToCDB(writebackUnit, headROBEntry);
    //     numInstsToCommit++;
    // }

    // TODO: Cleanup

    // if CDB still has space available, try to add more instructions to commit
    if (writebackUnit->cdbsUsed < writebackUnit->NB) {

        // printf("space remaining in CDB, attempting to add more instructions to commit\n");

        for (int i = 0; i < writebackUnit->NB - writebackUnit->cdbsUsed; i++) {
            ROBStatusTableEntry *nextHeadEntry = robTable->entries[(robTable->headEntryIndex + numInstsToCommit) % robTable->NR];

            if (nextHeadEntry->busy /*&& !nextHeadEntry->flushed*/ && nextHeadEntry->state == INST_STATE_WROTE_RESULT) {
                printf("next head of ROB (%i) is ready to commit, adding it to the CDB\n", nextHeadEntry->index);

                addROBEntryToCDB(writebackUnit, nextHeadEntry);
                numInstsToCommit++;
            } else {
                printf("ROB: %i not ready to commit\n", nextHeadEntry->index);
                break;
            }
        }
    }

    // check the produced results from the memory unit
    LSFUResult *currLSFUResult = getMemoryUnitCurrentResult(memUnit);
    if (currLSFUResult) {
        printf("destROB: %i, address: %i, fuType: %s, value: %f\n", currLSFUResult->destROB, currLSFUResult->resultAddr, fuTypeToString(currLSFUResult->fuType), currLSFUResult->loadValue);

        ROBStatusTableEntry *entry = robTable->entries[currLSFUResult->destROB];
        ROBWBInfo *info = writebackUnit->robWBInfoArr[currLSFUResult->destROB];

        if (info->producedBy != VALUE_FROM_NONE) {
            printf("error: destination ROB found in functional unit was already seen, this shouldn't happen...\n");
            // TOOD: return or exit?
        }

        info->producedBy = VALUE_FROM_FU; // actually need the value from mem???
        info->producingFUType = currLSFUResult->fuType;
        // info->intVal = currIntFUResult->result;
        if (info->producingFUType == FU_TYPE_LOAD) {
            info->floatVal = currLSFUResult->loadValue;
            info->valueType = VALUE_TYPE_FLOAT;
        }
        info->addr = currLSFUResult->resultAddr;

        // stall functional unit by default and remove the stall later if its result is placed on the CDB
        // functionalUnits->lsFU->isStalled = 1;
        memUnit->isStalledFromWB = 1;

    } else {
        printf("no result available from memory unit\n");
    }

    // check the produced results from INT functional unit
    IntFUResult *currIntFUResult = getCurrentIntFunctionalUnitResult(functionalUnits->intFU);
    if (currIntFUResult) {
        printf("destROB: %i, value: %i read from INT functional unit result\n", currIntFUResult->destROB, currIntFUResult->result);

        ROBStatusTableEntry *entry = robTable->entries[currIntFUResult->destROB];
        ROBWBInfo *info = writebackUnit->robWBInfoArr[currIntFUResult->destROB];

        // only perform actions if the destination rob table is busy (discards results heading to flushed ROB)
        // if (entry->busy) { // TODO: is this necessary anymore? since you already remove everything from all the units -> also in every other FU check

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
        // } else {
        //     printf("\tdestROB: %i is not busy, ignoring results\n", currIntFUResult->destROB);
        // }
    } else {
        printf("no result available from INT functional unit\n");
    }

    // check the produced results from FPAdd functional unit
    FloatFUResult *currFPAddFUResult = getCurrentFPFunctionalUnitResult(functionalUnits->fpAddFU);
    if (currFPAddFUResult) {
        printf("destROB: %i, value: %f read from FPAdd functional unit result\n", currFPAddFUResult->destROB, currFPAddFUResult->result);

        ROBStatusTableEntry *entry = robTable->entries[currFPAddFUResult->destROB];
        ROBWBInfo *info = writebackUnit->robWBInfoArr[currFPAddFUResult->destROB];

        // only perform actions if the destination rob table is busy (discards results heading to flushed ROB)
        // if (entry->busy) {

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
        // } else {
        //     printf("\tdestROB: %i is not busy, ignoring results\n", currFPAddFUResult->destROB);
        // }
    } else {
        printf("no result available from FPAdd functional unit\n");
    }

    // check the produced results from FPMul functional unit
    FloatFUResult *currFpMulFUResult = getCurrentFPFunctionalUnitResult(functionalUnits->fpMulFU);
    if (currFpMulFUResult) {
        printf("destROB: %i, value: %f read from FPMul functional unit result\n", currFpMulFUResult->destROB, currFpMulFUResult->result);

        ROBStatusTableEntry *entry = robTable->entries[currFpMulFUResult->destROB];
        ROBWBInfo *info = writebackUnit->robWBInfoArr[currFpMulFUResult->destROB];

        // only perform actions if the destination rob table is busy (discards results heading to flushed ROB)
        // if (entry->busy) {

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
        // } else {
        //     printf("\tdestROB: %i is not busy, ignoring results\n", currFpMulFUResult->destROB);
        // }
    } else {
        printf("no result available from FPMul functional unit\n");
    }

    // check the produced results from FPMul functional unit
    FloatFUResult *currFpDivFUResult = getCurrentFPFunctionalUnitResult(functionalUnits->fpDivFU);
    if (currFpDivFUResult) {
        printf("destROB: %i, value: %f read from FPDiv functional unit result\n", currFpDivFUResult->destROB, currFpDivFUResult->result);

        ROBStatusTableEntry *entry = robTable->entries[currFpDivFUResult->destROB];
        ROBWBInfo *info = writebackUnit->robWBInfoArr[currFpDivFUResult->destROB];

        // only perform actions if the destination rob table is busy (discards results heading to flushed ROB)
        // if (entry->busy) {

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
        // } else {
        //     printf("\tdestROB: %i is not busy, ignoring results\n", currFpDivFUResult->destROB);
        // }
    } else {
        printf("no result available from FPDiv functional unit\n");
    }

    // check the produced results from BU functional unit
    BUFUResult *currBUFUResult = getCurrentBUFunctionalUnitResult(functionalUnits->buFU);
    if (currBUFUResult) {
        printf("add BU result to CDB: isBranchTaken: %i, effective address: %i read from BU functional\n", currBUFUResult->isBranchTaken, currBUFUResult->effAddr);

        ROBStatusTableEntry *entry = robTable->entries[currBUFUResult->destROB];
        
        // only perform actions if the destination rob table is busy (discards results heading to flushed ROB)
        if (/*entry->busy &&*/ writebackUnit->cdbsUsed < writebackUnit->NB) {

            // always try to place BU result on CDB if result is available
            CDB *cdb = writebackUnit->cdbs[writebackUnit->cdbsUsed++];
            cdb->producingFUType = FU_TYPE_BU;
            cdb->destPhysReg = PHYS_REG_NONE;
            cdb->valueType = VALUE_TYPE_NONE;
            cdb->producedBy = VALUE_FROM_FU;
            cdb->robIndex = currBUFUResult->destROB;
            cdb->addr = currBUFUResult->effAddr;
            cdb->buTookBranch = currBUFUResult->isBranchTaken;

            functionalUnits->buFU->isStalled = 0;

        } else {
            printf("\tdestROB: %i is not busy, ignoring results\n", currBUFUResult->destROB);
            functionalUnits->buFU->isStalled = 1;
        }

    } else {
        printf("no result available from BU functional unit\n");
    }

    // TODO: need to add every other functional unit here as well - done?

    // increase the robsNeededBy counters for every functional unit's reservation stations
    updateWritebackUnitWaitingForROB(writebackUnit, resStationTable->intEntries, resStationTable->numIntStations);
    updateWritebackUnitWaitingForROB(writebackUnit, resStationTable->loadEntries, resStationTable->numLoadStations);
    updateWritebackUnitWaitingForROB(writebackUnit, resStationTable->storeEntries, resStationTable->numStoreStations);
    updateWritebackUnitWaitingForROB(writebackUnit, resStationTable->fpAddEntries, resStationTable->numFPAddStations);
    updateWritebackUnitWaitingForROB(writebackUnit, resStationTable->fpMulEntries, resStationTable->numFPMulStations);
    updateWritebackUnitWaitingForROB(writebackUnit, resStationTable->fpDivEntries, resStationTable->numFPDivStations);
    updateWritebackUnitWaitingForROB(writebackUnit, resStationTable->buEntries, resStationTable->numBUStations);
    updateWritebackUnitWaitingForROB(writebackUnit, resStationTable->loadEntries, resStationTable->numLoadStations);
    updateWritebackUnitWaitingForROB(writebackUnit, resStationTable->storeEntries, resStationTable->numStoreStations);

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
            cdb->addr = info->addr;
            
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

    // iterate over every entry in the CDB
    for (int i = 0; i < writebackUnit->cdbsUsed; i++) {
        CDB *cdb = writebackUnit->cdbs[i];

        // check if the value sent on the CDB came from a functional unit
        if (cdb->producedBy == VALUE_FROM_FU) {

            printf("CDB: %i contains value from a functional unit, updating ROB: %i to WROTE_RESULT and associated reservation station to busy = 0\n", i, cdb->robIndex);

            // update the ROB status table and reservation stations that are waiting for the result
            ROBStatusTableEntry *robStatusEntry = robTable->entries[cdb->robIndex];
            robStatusEntry->state = INST_STATE_WROTE_RESULT;

            if (cdb->valueType == VALUE_TYPE_INT) {
                robStatusEntry->intValue = cdb->intVal;
                sendIntUpdateToResStationStatusTable(resStationTable, cdb->robIndex, cdb->intVal, 1);
            } else if (cdb->valueType == VALUE_TYPE_FLOAT) {
                robStatusEntry->floatValue = cdb->floatVal;
                sendFloatUpdateToResStationStatusTable(resStationTable, cdb->robIndex, cdb->floatVal, 1);
            }

            // update reservation station status table to free reservation station that produced the result
            ResStationStatusTableEntry *resStationEntry = resStationEntryForFunctionalUnitWithDestROB(resStationTable, cdb->producingFUType, cdb->robIndex);
            resStationEntry->busy = 0;

            // don't actually stall the functional units (among other things)
            if (cdb->producingFUType == FU_TYPE_INT) {
                functionalUnits->intFU->isStalled = 0;
            } else if (cdb->producingFUType == FU_TYPE_LOAD) {
                memUnit->isStalledFromWB = 0;
            } else if (cdb->producingFUType == FU_TYPE_STORE) {
                memUnit->isStalledFromWB = 0;

                robStatusEntry->addr = cdb->addr;
                robStatusEntry->floatValue = resStationEntry->vjFloat;

                // forward the float value to the memory unit for the subsequent load to use
                // this works (and doesn't need to do any checking for if there are two stores to the same address) since the only time a load will see the forwarded result
                forwardDataToMemoryUnit(memUnit, robStatusEntry->floatValue, robStatusEntry->addr);

            } else if (cdb->producingFUType == FU_TYPE_FPADD) {
                functionalUnits->fpAddFU->isStalled = 0;
            } else if (cdb->producingFUType == FU_TYPE_FPMUL) {
                functionalUnits->fpMulFU->isStalled = 0;
            } else if (cdb->producingFUType == FU_TYPE_FPDIV) {
                functionalUnits->fpDivFU->isStalled = 0;
            } else if (cdb->producingFUType == FU_TYPE_BU) {
                // do not need to remove stall from BU functional unit as it is never added in the first place, since it is always added to the CDB if a result is available
                // TODO: hmm
                functionalUnits->buFU->isStalled = 0;
                robStatusEntry->addr = cdb->addr;
            } else {
                printf("error: invalid FUType when removing stall from functional unit for CDB: %i\n", i);
            }

            // TODO: again this for ever functional unit - done?
        
        // value came from the ROB (instruction will commit)
        } else if (cdb->producedBy == VALUE_FROM_ROB) {

            // update ROB status table
            ROBStatusTableEntry *robStatusEntry = robTable->entries[cdb->robIndex];
            robStatusEntry->busy = 0;
            robStatusEntry->state = INST_STATE_COMMIT;

            // if ROB was flushed, do not actually do anything
            // TODO: remove this check
            // if (robStatusEntry->flushed) {
            //     printf("ROB: %i was flushed, do not actually commit values\n", robStatusEntry->index);
            //     robStatusEntry->flushed = 0;

            //     // update free list
            //     popOldPhysicalRegisterMappingForReg(decodeUnit, robStatusEntry->destReg, 0);
            // } else {

                if (cdb->producingFUType == FU_TYPE_BU) {
                    printf("CDB: %i contains value from ROB, updating ROB: %i to COMMITED and updating branch predictor and ROB\n", i, cdb->robIndex);

                    printf("\n\n\ncdb->addr: %i, predictNextPC: %i\n\n\n", cdb->addr, predictNextPC(branchPredictor, robStatusEntry->inst->addr));

                    // check if the branch prediction was correct
                    if (cdb->addr == predictNextPC(branchPredictor, robStatusEntry->inst->addr)) { 
                        
                        // update the state of the branch predictor that the speculated branch was correct
                        updateBranchPredictor(branchPredictor, 1);

                        // update BTB
                        updateBTBEntry(branchPredictor, robStatusEntry->inst->addr, cdb->addr);

                    // branch prediction was incorrect
                    } else {
                        
                        // update the state of the branch predictor that the speculated branch was incorrect
                        updateBranchPredictor(branchPredictor, 0);

                        // update BTB
                        updateBTBEntry(branchPredictor, robStatusEntry->inst->addr, cdb->addr);

                        printFreeList(decodeUnit);
                        printMapTable(decodeUnit);
                        printROBStatusTable(robTable);

                        // add all allocated physical registers back to the free list
                        for (int i = (robTable->headEntryIndex + 1) % robTable->NR; i != robTable->headEntryIndex; i = (i + 1) % robTable->NR) {
                            // int index = (robTable->NR + robTable->headEntryIndex - robCount) % robTable->NR;
                            ROBStatusTableEntry *robEntryToReset = robTable->entries[i];
                            Instruction *inst = robEntryToReset->inst;

                            printf("rob entry to reset: %i\n", i);
                            if (inst) {
                                enum InstructionType instType = inst->type;

                                if (instType == ADDI || instType == ADD || instType == SLT || instType == FLD 
                                    || instType == FADD || instType == FSUB || instType == FDIV || instType == FMUL ) {
                                    
                                    // printf("rob entry to reset: %i for inst: %s\n", index, robEntryToReset->inst->fullStr);
                                    // printf("rob entry: %p\n", robEntryToReset);
                                    // // prin/tf()
                                    // printf("rob inst: %p\n", robEntryToReset->inst);
                                    // printf("rob inst: %s\n", robEntryToReset->inst->fullStr);


                                    if (robEntryToReset->busy && robEntryToReset->destReg) {
                                        popNewPhysicalRegisterMappingForReg(decodeUnit, robEntryToReset->destReg, 0);
                                    }
                                }
                            }

                        }

                        flushDecodeQueue(decodeUnit);
                        flushFetchBuffer(fetchUnit);

                        printFreeList(decodeUnit);
                        printMapTable(decodeUnit);

                        // // flush the ROB
                        flushROB(robTable);

                        // flush the register status table
                        flushRegisterStatusTable(regTable);
                        
                        // flush the reservation station status table
                        flushResStationStatusTable(resStationTable);

                        // write the correct PC value to the register file
                        writeRegisterFileInt(registerFile, PHYS_REG_PC, cdb->addr);

                        // reset functional units
                        flushBUFunctionalUnit(functionalUnits->buFU);
                        flushIntFunctionalUnit(functionalUnits->intFU);
                        flushFPFunctionalUnit(functionalUnits->fpAddFU);
                        flushFPFunctionalUnit(functionalUnits->fpMulFU);
                        flushFPFunctionalUnit(functionalUnits->fpDivFU);
                        flushLSFunctionalUnit(functionalUnits->lsFU);
                        flushMemUnit(memUnit);

                        // TODO: flush remaining functional units when you add them

                        // do not process any more cdb values

                        printf("\tupdating ROB head from: %i to ", robTable->headEntryIndex );
                        robTable->headEntryIndex = (robStatusEntry->index+1) % robTable->NR;
                        printf(" %i\n", robTable->headEntryIndex);
                        break;                        
                    }

                } else {
                    printf("CDB: %i contains value from ROB, updating ROB: %i to COMMITED and writing the result to register file\n", i, cdb->robIndex);


                    // update reservation stations with value
                    if (cdb->valueType == VALUE_TYPE_INT) {
                        sendIntUpdateToResStationStatusTable(resStationTable, cdb->robIndex, cdb->intVal, 1);
                    } else if (cdb->valueType == VALUE_TYPE_FLOAT) {
                        sendFloatUpdateToResStationStatusTable(resStationTable, cdb->robIndex, cdb->floatVal, 1);
                    }

                    // update register status table
                    RegisterStatusTableEntry *regStatusEntry = registerStatusTableEntryForReg(regTable, robStatusEntry->destReg);

                    // reset the register status table entry if the committed instruction's destination ROB matches the current entry's ROB
                    if (regStatusEntry && regStatusEntry->robIndex == robStatusEntry->index) {
                        regStatusEntry->robIndex = -1;
                    }

                    // only pop if the free list contains an older value
                    
                    // the issue was caused because the value was popped off the free list (and it wasn't prevented because another instruction was issued for that register, which then got removed due to branch misprediction, resulting in having 0 mappings for the committed value)
                    // how do i solve this then....
                    // check the rob status table?
                    // if one of the values has not been committed?
                    
                    // do not try to pop BNE and FSD destination registers
                    if (robStatusEntry->destReg) {
                        popOldPhysicalRegisterMappingForReg(decodeUnit, robTable, robStatusEntry->destReg, 1);
                    }

                    // update register file
                    if (cdb->valueType == VALUE_TYPE_INT) {
                        printf("\tcommiting: %i to register: %s\n", cdb->intVal, physicalRegisterNameToString(cdb->destPhysReg));
                        writeRegisterFileInt(registerFile, cdb->destPhysReg, cdb->intVal);
                    } else if (cdb->valueType == VALUE_TYPE_FLOAT) {
                        printf("\tcommiting: %f to register: %s\n", cdb->floatVal, physicalRegisterNameToString(cdb->destPhysReg));
                        writeRegisterFileFloat(registerFile, cdb->destPhysReg, cdb->floatVal);
                    }

                    // update memory if committing a store
                    if (cdb->producingFUType == FU_TYPE_STORE) {
                        printf("\twriting %f to memory address: %i\n", cdb->floatVal, cdb->addr);
                        writeFloatToDataCache(dataCache, cdb->addr, cdb->floatVal);
                    }
                }
            // }

            printf("\tupdating ROB head from: %i to ", robTable->headEntryIndex );
            robTable->headEntryIndex = (robStatusEntry->index+1) % robTable->NR;
            printf(" %i\n", robTable->headEntryIndex);
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