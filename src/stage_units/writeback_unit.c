
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../misc/misc.h"
#include "../status_tables/status_tables.h"
#include "../functional_units/functional_units.h"
#include "../memory/memory.h"
#include "../branch_prediction/branch_predictor.h"
#include "mem_unit.h"
#include "fetch_unit.h"
#include "decode_unit.h"
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
void printWritebackUnitCDBs(WritebackUnit *writebackUnit) {
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

    // place branch instruction addresses on the CDB
    if (entry->fuType == FU_TYPE_BU) {
        cdb->addr = entry->addr;
    
    // place load/store information on the CDB
    } else if (entry->fuType == FU_TYPE_STORE || entry->fuType == FU_TYPE_LOAD) {
        cdb->addr = entry->addr;
        cdb->floatVal = entry->floatValue;

    // place int or float value on the CDB
    } else {
        if (cdb->valueType == VALUE_TYPE_INT) {
            cdb->intVal = entry->intValue;
        } else if (cdb->valueType == VALUE_TYPE_FLOAT) {
            cdb->floatVal = entry->floatValue;
        } else {
            #ifdef ENABLE_DEBUG_LOG
            printf("error: ROB: %i to commit has an invalid value type, this shouldn't happen\n", entry->index);
            #endif
        }
    }
}

// perform writeback unit operations during a cycle
void cycleWritebackUnit(WritebackUnit *writebackUnit, FetchUnit *fetchUnit, DecodeUnit *decodeUnit, MemoryUnit *memUnit, StatusTables *statusTables, 
    FunctionalUnits *functionalUnits, RegisterFile *registerFile, DataCache *dataCache, BranchPredictor *branchPredictor, StallStats *stallStats) {

    printf_DEBUG(("\nperforming writeback unit operations...\n"));

    // make status tables easier to access
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

    // prioritize placing ROB entries on the CDB for commit
    if (writebackUnit->cdbsUsed < writebackUnit->NB) {

        for (int i = 0; i < writebackUnit->NB - writebackUnit->cdbsUsed; i++) {
            ROBStatusTableEntry *nextHeadEntry = robTable->entries[(robTable->headEntryIndex + numInstsToCommit) % robTable->NR];

            // check if next head ROB is read to commit
            if (nextHeadEntry->busy && nextHeadEntry->state == INST_STATE_WROTE_RESULT) {
                #ifdef ENABLE_DEBUG_LOG
                printf("next head of ROB (%i) is ready to commit, adding it to the CDB\n", nextHeadEntry->index);
                #endif

                addROBEntryToCDB(writebackUnit, nextHeadEntry);
                numInstsToCommit++;
            } else {
                #ifdef ENABLE_DEBUG_LOG
                printf("ROB: %i not ready to commit\n", nextHeadEntry->index);
                #endif

                break;
            }
        }
    }

    // check the produced results from the memory unit
    LSFUResult *currLSFUResult = getMemoryUnitCurrentResult(memUnit);
    if (currLSFUResult) {
        #ifdef ENABLE_DEBUG_LOG
        printf("destROB: %i, address: %i, fuType: %s, value: %f\n", currLSFUResult->destROB, currLSFUResult->resultAddr, fuTypeToString(currLSFUResult->fuType), currLSFUResult->loadValue);
        #endif

        ROBStatusTableEntry *entry = robTable->entries[currLSFUResult->destROB];
        ROBWBInfo *info = writebackUnit->robWBInfoArr[currLSFUResult->destROB];
        
        #ifdef ENABLE_DEBUG_LOG
        if (info->producedBy != VALUE_FROM_NONE) {
            printf("error: destination ROB found in functional unit was already seen, this shouldn't happen...\n");
        }
        #endif

        info->producedBy = VALUE_FROM_FU;
        info->producingFUType = currLSFUResult->fuType;
        info->addr = currLSFUResult->resultAddr;

        if (info->producingFUType == FU_TYPE_LOAD) {
            info->floatVal = currLSFUResult->loadValue;
            info->valueType = VALUE_TYPE_FLOAT;
        }

        // stall functional unit by default and remove the stall later if its result is placed on the CDB
        memUnit->isStalledFromWB = 1;

    } else {
        printf_DEBUG(("no result available from memory unit\n"));
    }

    // check the produced results from INT functional unit
    IntFUResult *currIntFUResult = getCurrentIntFunctionalUnitResult(functionalUnits->intFU);
    if (currIntFUResult) {
        #ifdef ENABLE_DEBUG_LOG
        printf("destROB: %i, value: %i read from INT functional unit result\n", currIntFUResult->destROB, currIntFUResult->result);
        #endif

        ROBStatusTableEntry *entry = robTable->entries[currIntFUResult->destROB];
        ROBWBInfo *info = writebackUnit->robWBInfoArr[currIntFUResult->destROB];

        #ifdef ENABLE_DEBUG_LOG
        if (info->producedBy != VALUE_FROM_NONE) {
            printf("error: destination ROB found in functional unit was already seen, this shouldn't happen...\n");
        }
        #endif

        info->producedBy = VALUE_FROM_FU;
        info->producingFUType = FU_TYPE_INT;
        info->intVal = currIntFUResult->result;
        info->valueType = VALUE_TYPE_INT;

        // stall functional unit by default and remove the stall later if its result is placed on the CDB
        functionalUnits->intFU->isStalled = 1;
    } else {
        printf_DEBUG(("no result available from INT functional unit\n"));
    }

    // check the produced results from FPAdd functional unit
    FloatFUResult *currFPAddFUResult = getCurrentFPFunctionalUnitResult(functionalUnits->fpAddFU);
    if (currFPAddFUResult) {
        #ifdef ENABLE_DEBUG_LOG
        printf("destROB: %i, value: %f read from FPAdd functional unit result\n", currFPAddFUResult->destROB, currFPAddFUResult->result);
        #endif

        ROBStatusTableEntry *entry = robTable->entries[currFPAddFUResult->destROB];
        ROBWBInfo *info = writebackUnit->robWBInfoArr[currFPAddFUResult->destROB];

        #ifdef ENABLE_DEBUG_LOG
        if (info->producedBy != VALUE_FROM_NONE) {
            printf("error: destination ROB found in functional unit was already seen, this shouldn't happen...\n");
        }
        #endif

        info->producedBy = VALUE_FROM_FU;
        info->producingFUType = FU_TYPE_FPADD;
        info->floatVal = currFPAddFUResult->result;
        info->valueType = VALUE_TYPE_FLOAT;

        // stall functional unit by default and remove the stall later if its result is placed on the CDB
        functionalUnits->fpAddFU->isStalled = 1;
    } else {
        printf_DEBUG(("no result available from FPAdd functional unit\n"));
    }

    // check the produced results from FPMul functional unit
    FloatFUResult *currFpMulFUResult = getCurrentFPFunctionalUnitResult(functionalUnits->fpMulFU);
    if (currFpMulFUResult) {
        #ifdef ENABLE_DEBUG_LOG
        printf("destROB: %i, value: %f read from FPMul functional unit result\n", currFpMulFUResult->destROB, currFpMulFUResult->result);
        #endif

        ROBStatusTableEntry *entry = robTable->entries[currFpMulFUResult->destROB];
        ROBWBInfo *info = writebackUnit->robWBInfoArr[currFpMulFUResult->destROB];

        #ifdef ENABLE_DEBUG_LOG
        if (info->producedBy != VALUE_FROM_NONE) {
            printf("error: destination ROB found in functional unit was already seen, this shouldn't happen...\n");
        }
        #endif

        info->producedBy = VALUE_FROM_FU;
        info->producingFUType = FU_TYPE_FPMUL;
        info->floatVal = currFpMulFUResult->result;
        info->valueType = VALUE_TYPE_FLOAT;

        // stall functional unit by default and remove the stall later if its result is placed on the CDB
        functionalUnits->fpMulFU->isStalled = 1;
    } else {
        printf_DEBUG(("no result available from FPMul functional unit\n"));
    }

    // check the produced results from FPMul functional unit
    FloatFUResult *currFpDivFUResult = getCurrentFPFunctionalUnitResult(functionalUnits->fpDivFU);
    if (currFpDivFUResult) {
        #ifdef ENABLE_DEBUG_LOG
        printf("destROB: %i, value: %f read from FPDiv functional unit result\n", currFpDivFUResult->destROB, currFpDivFUResult->result);
        #endif

        ROBStatusTableEntry *entry = robTable->entries[currFpDivFUResult->destROB];
        ROBWBInfo *info = writebackUnit->robWBInfoArr[currFpDivFUResult->destROB];

        #ifdef ENABLE_DEBUG_LOG
        if (info->producedBy != VALUE_FROM_NONE) {
            printf("error: destination ROB found in functional unit was already seen, this shouldn't happen...\n");
        }
        #endif

        info->producedBy = VALUE_FROM_FU;
        info->producingFUType = FU_TYPE_FPDIV;
        info->floatVal = currFpDivFUResult->result;
        info->valueType = VALUE_TYPE_FLOAT;

        // stall functional unit by default and remove the stall later if its result is placed on the CDB
        functionalUnits->fpDivFU->isStalled = 1;
    } else {
        printf_DEBUG(("no result available from FPDiv functional unit\n"));
    }

    // check the produced results from BU functional unit
    BUFUResult *currBUFUResult = getCurrentBUFunctionalUnitResult(functionalUnits->buFU);
    if (currBUFUResult) {
        #ifdef ENABLE_DEBUG_LOG
        printf("add BU result to CDB: isBranchTaken: %i, effective address: %i read from BU functional\n", currBUFUResult->isBranchTaken, currBUFUResult->effAddr);
        #endif

        ROBStatusTableEntry *entry = robTable->entries[currBUFUResult->destROB];
        
        // always try to place BU result on CDB if there is space
        if (writebackUnit->cdbsUsed < writebackUnit->NB) {
            
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
            #ifdef ENABLE_DEBUG_LOG
            printf("\tdestROB: %i is not busy, ignoring results\n", currBUFUResult->destROB);
            #endif
            functionalUnits->buFU->isStalled = 1;
        }

    } else {
        printf_DEBUG(("no result available from BU functional unit\n"));
    }

    // increase the robsNeededBy counters for every functional unit's reservation stations
    // the more operands that are waiting for a ROB, the higher priority it has to be placed on the CDB
    updateWritebackUnitWaitingForROB(writebackUnit, resStationTable->intEntries, resStationTable->numIntStations);
    updateWritebackUnitWaitingForROB(writebackUnit, resStationTable->loadEntries, resStationTable->numLoadStations);
    updateWritebackUnitWaitingForROB(writebackUnit, resStationTable->storeEntries, resStationTable->numStoreStations);
    updateWritebackUnitWaitingForROB(writebackUnit, resStationTable->fpAddEntries, resStationTable->numFPAddStations);
    updateWritebackUnitWaitingForROB(writebackUnit, resStationTable->fpMulEntries, resStationTable->numFPMulStations);
    updateWritebackUnitWaitingForROB(writebackUnit, resStationTable->fpDivEntries, resStationTable->numFPDivStations);
    updateWritebackUnitWaitingForROB(writebackUnit, resStationTable->buEntries, resStationTable->numBUStations);
    updateWritebackUnitWaitingForROB(writebackUnit, resStationTable->loadEntries, resStationTable->numLoadStations);
    updateWritebackUnitWaitingForROB(writebackUnit, resStationTable->storeEntries, resStationTable->numStoreStations);

    #ifdef ENABLE_DEBUG_LOG
    printWritebackUnitROBInfo(writebackUnit);
    #endif

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

        // no more operands are waiting for a ROB, nothing more to be placed on the CDB
        if (maxOpsROB == -1) {
            printf_DEBUG(("no ROB with maximum number of waiting operands found\n"));
            break;

        // a ROB was selected as having the highest number of operands waiting for it, write it on the CDB
        } else {
            

            ROBWBInfo *info = writebackUnit->robWBInfoArr[maxOpsROB];
            CDB *cdb = writebackUnit->cdbs[writebackUnit->cdbsUsed];

            // set CDB values
            cdb->producedBy = VALUE_FROM_FU;
            cdb->robIndex = maxOpsROB;
            cdb->producingFUType = info->producingFUType;
            cdb->addr = info->addr;
            
            // the value is from the INT functional unit
            if (info->valueType == VALUE_TYPE_INT) {
                cdb->intVal = info->intVal;
                cdb->valueType = VALUE_TYPE_INT;
                
                #ifdef ENABLE_DEBUG_LOG
                printf("added robIndex: %i (operandsWaiting: %i) intVal: %i to CDB: %i\n", cdb->robIndex, info->numOperandsWaiting, cdb->intVal, writebackUnit->cdbsUsed);
                #endif

            // the value is from some float functional unit
            } else if (info->valueType == VALUE_TYPE_FLOAT) {
                cdb->floatVal = info->floatVal;
                cdb->valueType = VALUE_TYPE_FLOAT;

                #ifdef ENABLE_DEBUG_LOG
                printf("added robIndex: %i (operandsWaiting: %i) floatVal: %f to CDB: %i\n", cdb->robIndex, info->numOperandsWaiting, cdb->floatVal, writebackUnit->cdbsUsed);
                #endif
            } else {
                #ifdef ENABLE_DEBUG_LOG
                printf("error: value type not found when adding value to CDB\n");
                #endif
            }

            writebackUnit->cdbsUsed++;
        }
    }

    // log the number of CDBs that were used
    stallStats->utilizedCDBs += writebackUnit->cdbsUsed;

    // iterate over every entry in the CDB
    for (int i = 0; i < writebackUnit->cdbsUsed; i++) {
        CDB *cdb = writebackUnit->cdbs[i];

        // check if the value sent on the CDB came from a functional unit - meaning it's state should be changed to WROTE_RESULT
        if (cdb->producedBy == VALUE_FROM_FU) {

            #ifdef ENABLE_DEBUG_LOG
            printf("CDB: %i contains value from a functional unit, updating ROB: %i to WROTE_RESULT and associated reservation station to busy = 0\n", i, cdb->robIndex);
            #endif

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
                forwardDataToMemoryUnit(memUnit, robStatusEntry->floatValue, robStatusEntry->addr);

            } else if (cdb->producingFUType == FU_TYPE_FPADD) {
                functionalUnits->fpAddFU->isStalled = 0;
            } else if (cdb->producingFUType == FU_TYPE_FPMUL) {
                functionalUnits->fpMulFU->isStalled = 0;
            } else if (cdb->producingFUType == FU_TYPE_FPDIV) {
                functionalUnits->fpDivFU->isStalled = 0;
            } else if (cdb->producingFUType == FU_TYPE_BU) {
                functionalUnits->buFU->isStalled = 0;

                // set target address calculated in the branch unit
                robStatusEntry->addr = cdb->addr;
            } else {
                #ifdef ENABLE_DEBUG_LOG
                printf("error: invalid FUType when removing stall from functional unit for CDB: %i\n", i);
                #endif
            }
        
        // value came from the ROB - updating instruction state to COMMIT
        } else if (cdb->producedBy == VALUE_FROM_ROB) {

            // update ROB status table
            ROBStatusTableEntry *robStatusEntry = robTable->entries[cdb->robIndex];
            robStatusEntry->busy = 0;
            robStatusEntry->state = INST_STATE_COMMIT;

            // a bne instruction is committing
            if (cdb->producingFUType == FU_TYPE_BU) {
                #ifdef ENABLE_DEBUG_LOG
                printf("CDB: %i contains value from ROB, updating ROB: %i to COMMITED and updating branch predictor and ROB\n", i, cdb->robIndex);
                printf("\ncdb->addr: %i, predictNextPC: %i\n", cdb->addr, predictNextPC(branchPredictor, robStatusEntry->inst->addr));
                #endif

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

                    #ifdef ENABLE_DEBUG_LOG
                    printFreeList(decodeUnit);
                    printMapTable(decodeUnit);
                    printROBStatusTable(robTable);
                    #endif

                    // add all allocated physical registers back to the free list
                    for (int i = (robTable->headEntryIndex + 1) % robTable->NR; i != robTable->headEntryIndex; i = (i + 1) % robTable->NR) {
                        
                        ROBStatusTableEntry *robEntryToReset = robTable->entries[i];
                        Instruction *inst = robEntryToReset->inst;

                        if (inst) {
                            enum InstructionType instType = inst->type;
                            
                            // do not pop entries for instructions that do not write to registers
                            if (instType != BNE && instType != FSD) {
                                
                                if (robEntryToReset->busy && robEntryToReset->destReg) {
                                    popNewPhysicalRegisterMappingForReg(decodeUnit, robEntryToReset->destReg, 0);
                                }
                            }
                        }
                    }

                    // reset buffers and queues
                    flushDecodeQueue(decodeUnit);
                    flushFetchBuffer(fetchUnit);

                    // flush the ROB
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

                    // update ROB head
                    robTable->headEntryIndex = (robStatusEntry->index+1) % robTable->NR;

                    // stop processing CDB values
                    break;                        
                }

            // commit non branch instruction
            } else {
                #ifdef ENABLE_DEBUG_LOG
                printf("CDB: %i contains value from ROB, updating ROB: %i to COMMITED and writing the result to register file\n", i, cdb->robIndex);
                #endif

                // update reservation stations with value
                if (cdb->valueType == VALUE_TYPE_INT) {
                    sendIntUpdateToResStationStatusTable(resStationTable, cdb->robIndex, cdb->intVal, 1);
                } else if (cdb->valueType == VALUE_TYPE_FLOAT) {
                    sendFloatUpdateToResStationStatusTable(resStationTable, cdb->robIndex, cdb->floatVal, 1);
                }

                // get the reservation station associatied with the ROB
                RegisterStatusTableEntry *regStatusEntry = registerStatusTableEntryForReg(regTable, robStatusEntry->destReg);

                // reset the register status table entry if the committed instruction's destination ROB matches the current entry's ROB
                if (regStatusEntry && regStatusEntry->robIndex == robStatusEntry->index) {
                    regStatusEntry->robIndex = -1;
                }

                // do not try to pop BNE and FSD destination registers (they do not contain destReg values)
                if (robStatusEntry->destReg) {
                    popOldPhysicalRegisterMappingForReg(decodeUnit, robTable, robStatusEntry->destReg, 1);
                }

                // update register file with int or float value
                if (cdb->valueType == VALUE_TYPE_INT) {
                    #ifdef ENABLE_DEBUG_LOG
                    printf("\tcommiting: %i to register: %s\n", cdb->intVal, physicalRegisterNameToString(cdb->destPhysReg));
                    #endif 

                    writeRegisterFileInt(registerFile, cdb->destPhysReg, cdb->intVal);
                } else if (cdb->valueType == VALUE_TYPE_FLOAT) {
                    #ifdef ENABLE_DEBUG_LOG
                    printf("\tcommiting: %f to register: %s\n", cdb->floatVal, physicalRegisterNameToString(cdb->destPhysReg));
                    #endif

                    writeRegisterFileFloat(registerFile, cdb->destPhysReg, cdb->floatVal);
                }

                // update memory if committing a store
                if (cdb->producingFUType == FU_TYPE_STORE) {
                    #ifdef ENABLE_DEBUG_LOG
                    printf("\twriting %f to memory address: %i\n", cdb->floatVal, cdb->addr);
                    #endif 

                    writeFloatToDataCache(dataCache, cdb->addr, cdb->floatVal);
                }
            }
        
            // update the ROB head
            robTable->headEntryIndex = (robStatusEntry->index+1) % robTable->NR;
        }
    }
}