
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../misc/misc.h"
#include "../memory/memory.h"
#include "../functional_units/functional_units.h"
#include "register_table.h"
#include "res_station_table.h"

// returns a new reservation station status table entry allocated on the heap
ResStationStatusTableEntry *newResStationStatusTableEntry() {
    ResStationStatusTableEntry *entry = malloc(sizeof(ResStationStatusTableEntry));

    entry->fuType = FU_TYPE_NONE;
    entry->resStationIndex = -1;
    entry->busy = 0;
    entry->op = FU_OP_NONE;
    entry->vjInt = 0;
    entry->vjFloat = 0;
    entry->vjIsAvailable = 0;
    entry->vkInt = 0;
    entry->vkFloat = 0;
    entry->vkIsAvailable = 0;
    entry->justGotOperandFromCDB = 0;
    entry->qj = -1;
    entry->qk = -1;
    entry->dest = -1;
    entry->addr = -1;
    entry->buOffset = 0;

    return entry;
}

// initialize the reservation station status table
void initResStationStatusTable(ResStationStatusTable *resStationTable) {

    // declare the number of res stations for each functional unit
    resStationTable->numIntStations = 4;
    resStationTable->numLoadStations = 2;
    resStationTable->numStoreStations = 2;
    resStationTable->numFPAddStations = 3;
    resStationTable->numFPMulStations = 3;
    resStationTable->numFPDivStations = 2;
    resStationTable->numBUStations = 2;

    // allocate arrays to store reservation station status table entries for each functional unit
    resStationTable->intEntries = malloc(resStationTable->numIntStations * sizeof(ResStationStatusTableEntry *));
    resStationTable->loadEntries = malloc(resStationTable->numLoadStations * sizeof(ResStationStatusTableEntry *));
    resStationTable->storeEntries = malloc(resStationTable->numStoreStations * sizeof(ResStationStatusTableEntry *));
    resStationTable->fpAddEntries = malloc(resStationTable->numFPAddStations * sizeof(ResStationStatusTableEntry *));
    resStationTable->fpMulEntries = malloc(resStationTable->numFPMulStations * sizeof(ResStationStatusTableEntry *));
    resStationTable->fpDivEntries = malloc(resStationTable->numFPDivStations * sizeof(ResStationStatusTableEntry *));
    resStationTable->buEntries = malloc(resStationTable->numBUStations * sizeof(ResStationStatusTableEntry *));

    // initialize INT functional unit reservation station entries
    for (int i = 0; i < resStationTable->numIntStations; i++) {
        ResStationStatusTableEntry *entry = newResStationStatusTableEntry();
        
        entry->fuType = FU_TYPE_INT;
        entry->resStationIndex = i;

        resStationTable->intEntries[i] = entry;
    }

    // initialize load functional unit reservation station entries
    for (int i = 0; i < resStationTable->numLoadStations; i++) {
        ResStationStatusTableEntry *entry = newResStationStatusTableEntry();
        
        entry->fuType = FU_TYPE_LOAD;
        entry->resStationIndex = i;

        resStationTable->loadEntries[i] = entry;
    }

    // initialize store functional unit reservation station entries
    for (int i = 0; i < resStationTable->numStoreStations; i++) {
        ResStationStatusTableEntry *entry = newResStationStatusTableEntry();
        
        entry->fuType = FU_TYPE_STORE;
        entry->resStationIndex = i;

        resStationTable->storeEntries[i] = entry;
    }

    // initialize FPAdd functional unit reservation station entries
    for (int i = 0; i < resStationTable->numFPAddStations; i++) {
        ResStationStatusTableEntry *entry = newResStationStatusTableEntry();
        
        entry->fuType = FU_TYPE_FPADD;
        entry->resStationIndex = i;

        resStationTable->fpAddEntries[i] = entry;
    }

    // initialize FPMul functional unit reservation station entries
    for (int i = 0; i < resStationTable->numFPMulStations; i++) {
        ResStationStatusTableEntry *entry = newResStationStatusTableEntry();
        
        entry->fuType = FU_TYPE_FPMUL;
        entry->resStationIndex = i;

        resStationTable->fpMulEntries[i] = entry;
    }

    // initialize FPDiv functional unit reservation station entries
    for (int i = 0; i < resStationTable->numFPDivStations; i++) {
        ResStationStatusTableEntry *entry = newResStationStatusTableEntry();
        
        entry->fuType = FU_TYPE_FPDIV;
        entry->resStationIndex = i;

        resStationTable->fpDivEntries[i] = entry;
    }

    // initialize BU functional unit reservation station entries
    for (int i = 0; i < resStationTable->numBUStations; i++) {
        ResStationStatusTableEntry *entry = newResStationStatusTableEntry();
        
        entry->fuType = FU_TYPE_BU;
        entry->resStationIndex = i;

        resStationTable->buEntries[i] = entry;
    }
}

// free any data elements of the reservation status table that are stored on the heap
void teardownResStationStatusTable(ResStationStatusTable *resStationTable) {
    if (resStationTable->intEntries) {
        for (int i = 0; i < resStationTable->numIntStations; i++) {
            free(resStationTable->intEntries[i]);
        }
        free(resStationTable->intEntries);
    }

    if (resStationTable->loadEntries) {
        for (int i = 0; i < resStationTable->numLoadStations; i++) {
            free(resStationTable->loadEntries[i]);
        }
        free(resStationTable->loadEntries);
    }

    if (resStationTable->storeEntries) {
        for (int i = 0; i < resStationTable->numStoreStations; i++) {
            free(resStationTable->storeEntries[i]);
        }
        free(resStationTable->storeEntries);
    }

    if (resStationTable->fpAddEntries) {
        for (int i = 0; i < resStationTable->numFPAddStations; i++) {
            free(resStationTable->fpAddEntries[i]);
        }
        free(resStationTable->fpAddEntries);
    }

    if (resStationTable->fpMulEntries) {
        for (int i = 0; i < resStationTable->numFPMulStations; i++) {
            free(resStationTable->fpMulEntries[i]);
        }
        free(resStationTable->fpMulEntries);
    }

    if (resStationTable->fpDivEntries) {
        for (int i = 0; i < resStationTable->numFPDivStations; i++) {
            free(resStationTable->fpDivEntries[i]);
        }
        free(resStationTable->fpDivEntries);
    }

    if (resStationTable->buEntries) {
        for (int i = 0; i < resStationTable->numBUStations; i++) {
            free(resStationTable->buEntries[i]);
        }
        free(resStationTable->buEntries);
    }
}

// returns the number of reservation stations for a given functional unit
int numResStationsForFunctionalUnit(ResStationStatusTable *resStationTable, int fuType) {
    
    if (fuType == FU_TYPE_INT) {
        return resStationTable->numIntStations;
    } else if (fuType == FU_TYPE_LOAD) {
        return resStationTable->numLoadStations;
    } else if (fuType == FU_TYPE_STORE) {
        return resStationTable->numStoreStations;
    } else if (fuType == FU_TYPE_FPADD) {
        return resStationTable->numFPAddStations;
    } else if (fuType == FU_TYPE_FPMUL) {
        return resStationTable->numFPMulStations;
    } else if (fuType == FU_TYPE_FPDIV) {
        return resStationTable->numFPDivStations;
    } else if (fuType == FU_TYPE_BU) {
        return resStationTable->numBUStations;
    } else {
        printf_DEBUG(("error: invalid FunctionalUnitType used while getting number of reservation stations...\n"));
        exit(1);
    }
}

// returns the reservation station entry list for a given functional unit
ResStationStatusTableEntry **resStationEntriesForFunctionalUnit(ResStationStatusTable *resStationTable, int fuType) {
    
    if (fuType == FU_TYPE_INT) {
        return resStationTable->intEntries;
    } else if (fuType == FU_TYPE_LOAD) {
        return resStationTable->loadEntries;
    } else if (fuType == FU_TYPE_STORE) {
        return resStationTable->storeEntries;
    } else if (fuType == FU_TYPE_FPADD) {
        return resStationTable->fpAddEntries;
    } else if (fuType == FU_TYPE_FPMUL) {
        return resStationTable->fpMulEntries;
    } else if (fuType == FU_TYPE_FPDIV) {
        return resStationTable->fpDivEntries;
    } else if (fuType == FU_TYPE_BU) {
        return resStationTable->buEntries;
    } else {
        printf_DEBUG(("error: invalid FunctionalUnitType used while get reservation station entries array...\n"));
        exit(1);
    }
}

// returns the reservation station entry in a given functional unit array which while write to a given ROB
ResStationStatusTableEntry *resStationEntryForFunctionalUnitWithDestROB(ResStationStatusTable *resStationTable, int fuType, int destROB) {

    ResStationStatusTableEntry **entries = resStationEntriesForFunctionalUnit(resStationTable, fuType);
    int numEntries = numResStationsForFunctionalUnit(resStationTable, fuType);
    
    for (int i = 0; i < numEntries; i++) {
        ResStationStatusTableEntry *entry = entries[i];
        
        if (entry->dest == destROB) {
            return entry;
        }
    }

    return NULL;
}

// returns the reservation station entry list for a given instruction
ResStationStatusTableEntry **resStationEntriesForInstruction(ResStationStatusTable *resStationTable, Instruction *inst) {

    enum InstructionType instType = inst->type;

    // search for a reservation station needed by the given instruction
    if (instType == ADD || instType == ADDI || instType == SLT) {
        return resStationTable->intEntries;
    } else if (instType == FLD) {
        return resStationTable->loadEntries;
    } else if (instType == FSD) {
        return resStationTable->storeEntries;
    } else if (instType == FADD || instType == FSUB) {
        return resStationTable->fpAddEntries;
    } else if (instType == FMUL) {
        return resStationTable->fpMulEntries;
    } else if (instType == FDIV) {
        return resStationTable->fpDivEntries;
    } else if (instType == BNE) {
        return resStationTable->buEntries;
    } else {
        printf_DEBUG(("got invalid instruction type while trying to get the reservation station entry array for an instruction, this should never happen..."));
        exit(1);
    }
}

// returns the index of a free INT reservation station if one is available, otherwise return -1
int indexForFreeResStation(ResStationStatusTable *resStationTable, int fuType) {

    // get the number of reservation stations and the entry array for the given functional unit type
    int numStations = numResStationsForFunctionalUnit(resStationTable, fuType);
    ResStationStatusTableEntry **entries = resStationEntriesForFunctionalUnit(resStationTable, fuType);
    
    // iterate over every reservation station for the given functional unit
    for (int i = 0; i < numStations; i++) {

        // check if the reservation station is not busy, meaning it is available
        if (!(entries[i]->busy)) {
            return i;
        }
    }

    return -1;
}

// if a reservation station needed for a given instruction is available, return it's index in the reservation stations array, otherwise return -1
int indexForFreeResStationForInstruction(ResStationStatusTable *resStationTable, Instruction *inst) {

    enum InstructionType instType = inst->type;

    // search for a reservation station needed by the given instruction
    if (instType == ADD || instType == ADDI || instType == SLT) {
        return indexForFreeResStation(resStationTable, FU_TYPE_INT);
    } else if (instType == FLD) {
        return indexForFreeResStation(resStationTable, FU_TYPE_LOAD);
    } else if (instType == FSD) {
        return indexForFreeResStation(resStationTable, FU_TYPE_STORE);
    } else if (instType == FADD || instType == FSUB) {
        return indexForFreeResStation(resStationTable, FU_TYPE_FPADD);
    } else if (instType == FMUL) {
        return indexForFreeResStation(resStationTable, FU_TYPE_FPMUL);
    } else if (instType == FDIV) {
        return indexForFreeResStation(resStationTable, FU_TYPE_FPDIV);
    } else if (instType == BNE) {
        return indexForFreeResStation(resStationTable, FU_TYPE_BU);
    } else {
        printf_DEBUG(("got invalid instruction type while trying to get the index of a free reservation station, this should never happen..."));
        exit(1);
    }

    return -1;
}

// returns 1 if there is a reservation station avaialble for the given instruction, 0 if not
int isFreeResStationForInstruction(ResStationStatusTable *resStationTable, Instruction *inst) {
    return indexForFreeResStationForInstruction(resStationTable, inst) != -1;
}

// returns the number of filled reservation stations for a given functional unit
int numBusyResStationsForFunctionalUnit(ResStationStatusTable *resStationTable, int fuType) {
    
    // get the number of reservation stations and the entry array for the given functional unit type
    int numStations = numResStationsForFunctionalUnit(resStationTable, fuType);
    ResStationStatusTableEntry **entries = resStationEntriesForFunctionalUnit(resStationTable, fuType);

    int numBusyStations = 0;

    // iterate over the current reservation station entries checking for ones that are busy
    for (int i = 0; i < numStations; i++) {
        if (entries[i]->busy) {
            numBusyStations++;
        }
    }

    return numBusyStations;
}

// helper method to set the availability of a given operand in a reservation station status table entry
void setResStationEntryOperandAvailability(ResStationStatusTableEntry *entry, RegisterStatusTable *regTable, RegisterFile *regFile, 
    int sourceNum, ArchRegister *reg, enum PhysicalRegisterName renamedReg, int resultType) {

    // get the current ROB that will write to the source architecture register
    int robIndex = getRegisterStatusTableEntryROBIndex(regTable, reg);

    // source is not associated with a ROB entry, read from register file
    if (robIndex == -1) {
        if (sourceNum == 1) {
            if (resultType == VALUE_TYPE_INT) {
                entry->vjInt = readRegisterFileInt(regFile, renamedReg);
            } else {
                entry->vjFloat = readRegisterFileFloat(regFile, renamedReg);
            }
            
            entry->vjIsAvailable = 1;
        } else {
            if (resultType == VALUE_TYPE_INT) {
                entry->vkInt = readRegisterFileInt(regFile, renamedReg);
            } else {
                entry->vkFloat = readRegisterFileFloat(regFile, renamedReg);
            }
            entry->vkIsAvailable = 1;
        }
        
    // source is (or will be) located in the ROB
    } else {
        if (sourceNum == 1) {
            entry->vjIsAvailable = 0;
            entry->qj = robIndex;
        } else {
            entry->vkIsAvailable = 0;
            entry->qk = robIndex;
        }
    }
}

// updates the reservation station status table for a given instruction
void addInstToResStation(ResStationStatusTable *resStationTable, RegisterStatusTable *regTable, RegisterFile *regFile, Instruction *inst, int destROB) {

    enum InstructionType instType = inst->type;
    int resStationIndex = indexForFreeResStationForInstruction(resStationTable, inst);
    ResStationStatusTableEntry *entry = resStationEntriesForInstruction(resStationTable, inst)[resStationIndex];

    entry->dest = destROB;
    entry->busy = 1;

    // add instructions that need the INT functional unit to the reservation station
    if (instType == ADDI || instType == ADD || instType == SLT) {
        
        // add ADDI instructions to the reservation station
        if (instType == ADDI) {    

            entry->op = FU_OP_ADD;

            // read first operand from register file or set source ROB 
            setResStationEntryOperandAvailability(entry, regTable, regFile, 1, inst->source1Reg, inst->source1PhysReg, VALUE_TYPE_INT);

            // manually do the second operand since its value is available within the instruction itself
            entry->vkInt = inst->imm;
            entry->vkIsAvailable = 1;
        
        // add ADD or SLT instructions to the reservation station
        } else {
            
            entry->op = instType == ADD ? FU_OP_ADD : FU_OP_SLT;

            // read operands from register file or set source ROB 
            setResStationEntryOperandAvailability(entry, regTable, regFile, 1, inst->source1Reg, inst->source1PhysReg, VALUE_TYPE_INT);
            setResStationEntryOperandAvailability(entry, regTable, regFile, 2, inst->source2Reg, inst->source2PhysReg, VALUE_TYPE_INT);
        }

    // add instructions that need the FPAdd functional unit to the reservation station
    } else if (instType == FADD || instType == FSUB || instType == FMUL || instType == FDIV) {

        // set the entry's add or sub operation for FADD and FSUB instructions
        if (instType == FADD || instType == FSUB) {
            entry->op = instType == FADD ? FU_OP_ADD : FU_OP_SUB;
        }

        // read operands from register file or set source ROB 
        setResStationEntryOperandAvailability(entry, regTable, regFile, 1, inst->source1Reg, inst->source1PhysReg, VALUE_TYPE_FLOAT);
        setResStationEntryOperandAvailability(entry, regTable, regFile, 2, inst->source2Reg, inst->source2PhysReg, VALUE_TYPE_FLOAT);
    
    // add instructions that need the load functional unit to the reservation station
    } else if (instType == FLD) {

        entry->addr = inst->imm;
        setResStationEntryOperandAvailability(entry, regTable, regFile, 2, inst->source2Reg, inst->source2PhysReg, VALUE_TYPE_INT);

    // add instructions that need the store functional unit to the reservation station
    } else if (instType == FSD) {

        // in reality stores use the destination field but having it be the source 2 simplified things for implementing this

        entry->addr = inst->imm;
        setResStationEntryOperandAvailability(entry, regTable, regFile, 1, inst->source1Reg, inst->source1PhysReg, VALUE_TYPE_FLOAT);
        setResStationEntryOperandAvailability(entry, regTable, regFile, 2, inst->source2Reg, inst->source2PhysReg, VALUE_TYPE_INT);
    
    // add instructions that need the BU functional unit to the reservation station
    } else if (instType == BNE) {

        entry->buOffset = inst->imm;
        entry->addr = inst->addr;

        setResStationEntryOperandAvailability(entry, regTable, regFile, 1, inst->source1Reg, inst->source1PhysReg, VALUE_TYPE_INT);
        setResStationEntryOperandAvailability(entry, regTable, regFile, 2, inst->source2Reg, inst->source2PhysReg, VALUE_TYPE_INT);
    }

    #ifdef ENABLE_DEBUG_LOG
    printf("added instruction: %p to reservation station: %i\n", inst, resStationIndex);
    #endif
}

// helper method to process int updates for any list of reservation station status table entries
void processIntUpdateForResStationEntries(ResStationStatusTableEntry **entries, int numEntries, int destROB, int result, int fromCDB) {

    // iterate over all entries of the reservation station
    for (int i = 0; i < numEntries; i++) {
        ResStationStatusTableEntry *entry = entries[i];

        // only update the entry's operands if they are currently available and the result ROB matches the source ROB
        if (entry->busy && !entry->vjIsAvailable && entry->qj == destROB) {
            #ifdef ENABLE_DEBUG_LOG
            printf("reservation station index: %i received int value: %i for vj\n", entry->resStationIndex, result);
            #endif

            entry->vjIsAvailable = 1;
            entry->vjInt = result;
            entry->qj = -1;
            entry->justGotOperandFromCDB = fromCDB;
        }

        if (entry->busy && !entry->vkIsAvailable && entry->qk == destROB) {
            #ifdef ENABLE_DEBUG_LOG
            printf("reservation station index: %i received int value: %i for vk\n", entry->resStationIndex, result);
            #endif

            entry->vkIsAvailable = 1;
            entry->vkInt = result;
            entry->qk = -1;
            entry->justGotOperandFromCDB = fromCDB;
        }
    }
}

// helper method to process float updates for any list of reservation station status table entries
void processFloatUpdateForResStationEntries(ResStationStatusTableEntry **entries, int numEntries, int destROB, float result, int fromCDB) {

    // iterate over the reservation station entries
    for (int i = 0; i < numEntries; i++) {
        ResStationStatusTableEntry *entry = entries[i];

        // only update the entry's operands if they are currently available and the result ROB matches the source ROB
        if (entry->busy && !entry->vjIsAvailable && entry->qj == destROB) {
            #ifdef ENABLE_DEBUG_LOG
            printf("reservation station index: %i received float value: %f for vj\n", entry->resStationIndex, result);
            #endif

            entry->vjIsAvailable = 1;
            entry->vjFloat = result;
            entry->qj = -1;
            entry->justGotOperandFromCDB = fromCDB;
        }

        if (entry->busy && !entry->vkIsAvailable && entry->qk == destROB) {
            #ifdef ENABLE_DEBUG_LOG
            printf("reservation station index: %i received forwarded float: %f for vk\n", entry->resStationIndex, result);
            #endif

            entry->vkIsAvailable = 1;
            entry->vkFloat = result;
            entry->qk = -1;
            entry->justGotOperandFromCDB = fromCDB;
        }
    }
}

// updates the operands of reservation stations waiting for an integer value
void sendIntUpdateToResStationStatusTable(ResStationStatusTable *resStationTable, int destROB, int result, int fromCDB) {

    #ifdef ENABLE_DEBUG_LOG
    printf("sending int result: %i robIndex: %i to reservation stations\n", result, destROB);
    #endif

    // forward int result to reservation stations that can use int registers
    processIntUpdateForResStationEntries(resStationTable->intEntries, resStationTable->numIntStations, destROB, result, fromCDB);
    processIntUpdateForResStationEntries(resStationTable->loadEntries, resStationTable->numLoadStations, destROB, result, fromCDB);
    processIntUpdateForResStationEntries(resStationTable->storeEntries, resStationTable->numStoreStations, destROB, result, fromCDB);
    processIntUpdateForResStationEntries(resStationTable->buEntries, resStationTable->numBUStations, destROB, result, fromCDB);
}

// updates the operands of reservation stations waiting for a float value
void sendFloatUpdateToResStationStatusTable(ResStationStatusTable *resStationTable, int destROB, float result, int fromCDB) {

    #ifdef ENABLE_DEBUG_LOG
    printf("sending float result: %f robIndex: %i to reservation stations\n", result, destROB);
    #endif

    // forward int result to reservation stations that can use int registers
    processFloatUpdateForResStationEntries(resStationTable->loadEntries, resStationTable->numLoadStations, destROB, result, fromCDB);
    processFloatUpdateForResStationEntries(resStationTable->storeEntries, resStationTable->numStoreStations, destROB, result, fromCDB);
    processFloatUpdateForResStationEntries(resStationTable->buEntries, resStationTable->numBUStations, destROB, result, fromCDB);
    processFloatUpdateForResStationEntries(resStationTable->fpAddEntries, resStationTable->numFPAddStations, destROB, result, fromCDB);
    processFloatUpdateForResStationEntries(resStationTable->fpMulEntries, resStationTable->numFPMulStations, destROB, result, fromCDB);
    processFloatUpdateForResStationEntries(resStationTable->fpDivEntries, resStationTable->numFPDivStations, destROB, result, fromCDB);
}

// prints the contents of the reservation station status table
void printResStationStatusTable(ResStationStatusTable *resStationTable) {

    printf("reservation station status table:\n");

    printf("INT FU reservation stations:\n");
    for (int i = 0; i < resStationTable->numIntStations; i++) {
        ResStationStatusTableEntry *entry = resStationTable->intEntries[i];
        
        printf("\tindex: %i, busy: %i, op: %s, vjInt: %i, vkInt: %i, vjIsAvail: %i, vkIsAvail: %i, qj: %i, qk: %i, destROB: %i\n", 
            entry->resStationIndex, entry->busy, fuOpToString(entry->op), entry->vjInt, entry->vkInt,
            entry->vjIsAvailable, entry->vkIsAvailable, entry->qj, entry->qk, entry->dest);
    }

    printf("FPAdd FU reservation stations:\n");
    for (int i = 0; i < resStationTable->numFPAddStations; i++) {
        ResStationStatusTableEntry *entry = resStationTable->fpAddEntries[i];
        
        printf("\tindex: %i, busy: %i, op: %s, vjFloat: %f, vkFloat: %f, vjIsAvail: %i, vkIsAvail: %i, qj: %i, qk: %i, destROB: %i\n", 
            entry->resStationIndex, entry->busy, fuOpToString(entry->op), entry->vjFloat, entry->vkFloat,
            entry->vjIsAvailable, entry->vkIsAvailable, entry->qj, entry->qk, entry->dest);
    }

    printf("FPMul FU reservation stations:\n");
    for (int i = 0; i < resStationTable->numFPMulStations; i++) {
        ResStationStatusTableEntry *entry = resStationTable->fpMulEntries[i];
        
        printf("\tindex: %i, busy: %i, op: %s, vjFloat: %f, vkFloat: %f, vjIsAvail: %i, vkIsAvail: %i, qj: %i, qk: %i, destROB: %i\n", 
            entry->resStationIndex, entry->busy, fuOpToString(entry->op), entry->vjFloat, entry->vkFloat,
            entry->vjIsAvailable, entry->vkIsAvailable, entry->qj, entry->qk, entry->dest);
    }

    printf("FPDiv FU reservation stations:\n");
    for (int i = 0; i < resStationTable->numFPDivStations; i++) {
        ResStationStatusTableEntry *entry = resStationTable->fpDivEntries[i];
        
        printf("\tindex: %i, busy: %i, op: %s, vjFloat: %f, vkFloat: %f, vjIsAvail: %i, vkIsAvail: %i, qj: %i, qk: %i, destROB: %i\n", 
            entry->resStationIndex, entry->busy, fuOpToString(entry->op), entry->vjFloat, entry->vkFloat,
            entry->vjIsAvailable, entry->vkIsAvailable, entry->qj, entry->qk, entry->dest);
    }

    printf("BU FU reservation stations:\n");
    for (int i = 0; i < resStationTable->numBUStations; i++) {
        ResStationStatusTableEntry *entry = resStationTable->buEntries[i];
        
        printf("\tindex: %i, busy: %i, op: %s, vjInt: %i, vkInt: %i, vjIsAvail: %i, vkIsAvail: %i, qj: %i, qk: %i, destROB: %i\n", 
            entry->resStationIndex, entry->busy, fuOpToString(entry->op), entry->vjInt, entry->vkInt,
            entry->vjIsAvailable, entry->vkIsAvailable, entry->qj, entry->qk, entry->dest);
    }

    printf("load FU reservation stations:\n");
    for (int i = 0; i < resStationTable->numLoadStations; i++) {
        ResStationStatusTableEntry *entry = resStationTable->loadEntries[i];
        
        printf("\tindex: %i, busy: %i, op: %s, vjFloat: %f, vkFloat: %f, vjIsAvail: %i, vkIsAvail: %i, qj: %i, qk: %i, destROB: %i\n", 
            entry->resStationIndex, entry->busy, fuOpToString(entry->op), entry->vjFloat, entry->vkFloat,
            entry->vjIsAvailable, entry->vkIsAvailable, entry->qj, entry->qk, entry->dest);
    }

    printf("store FU reservation stations:\n");
    for (int i = 0; i < resStationTable->numStoreStations; i++) {
        ResStationStatusTableEntry *entry = resStationTable->storeEntries[i];
        
        printf("\tindex: %i, busy: %i, op: %s, vjFloat: %f, vkFloat: %f, vjIsAvail: %i, vkIsAvail: %i, qj: %i, qk: %i, destROB: %i\n", 
            entry->resStationIndex, entry->busy, fuOpToString(entry->op), entry->vjFloat, entry->vkFloat,
            entry->vjIsAvailable, entry->vkIsAvailable, entry->qj, entry->qk, entry->dest);
    }
}

// sets all reservation stations to not busy
void flushResStationStatusTable(ResStationStatusTable *resStationTable) {
    
    printf_DEBUG(("flushing reservation station status table:\n"));

    // clear int functional unit reservation stations
    for (int i = 0; i < resStationTable->numIntStations; i++) {
        ResStationStatusTableEntry *entry = resStationTable->intEntries[i];
        entry->busy = 0;    
    }

    // clear fpadd functional unit reservation stations
    for (int i = 0; i < resStationTable->numFPAddStations; i++) {
        ResStationStatusTableEntry *entry = resStationTable->fpAddEntries[i];
        entry->busy = 0;
    }

    // clear fpmul functional unit reservation stations
    for (int i = 0; i < resStationTable->numFPMulStations; i++) {
        ResStationStatusTableEntry *entry = resStationTable->fpMulEntries[i];
        entry->busy = 0;
    }

    // clear fpdiv functional unit reservation stations
    for (int i = 0; i < resStationTable->numFPDivStations; i++) {
        ResStationStatusTableEntry *entry = resStationTable->fpDivEntries[i];
        entry->busy = 0;
    }

    // clear BU functional unit reservation stations
    for (int i = 0; i < resStationTable->numBUStations; i++) {
        ResStationStatusTableEntry *entry = resStationTable->buEntries[i];
        entry->busy = 0;
    }

    // clear load functional unit reservation stations
    for (int i = 0; i < resStationTable->numLoadStations; i++) {
        ResStationStatusTableEntry *entry = resStationTable->loadEntries[i];
        entry->busy = 0;        
    }

    // clear store functional unit reservation stations
    for (int i = 0; i < resStationTable->numStoreStations; i++) {
        ResStationStatusTableEntry *entry = resStationTable->storeEntries[i];
        entry->busy = 0;
    }
}