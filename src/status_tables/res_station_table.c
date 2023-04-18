
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../types/types.h"
#include "res_station_table.h"
#include "register_table.h"
#include "../memory/memory.h"
#include "../functional_units/functional_units.h"

// returns a new reservation station status table entry allocated on the heap
ResStationStatusTableEntry *newResStationStatusTableEntry() {
    ResStationStatusTableEntry *entry = malloc(sizeof(ResStationStatusTableEntry));

    entry->fuType = FUType_NONE;
    entry->resStationIndex = -1;
    entry->busy = 0;
    entry->op = FUOp_NONE;
    entry->vjInt = 0;
    entry->vjFloat = 0;
    entry->vjIsAvailable = 0;
    entry->vkInt = 0;
    entry->vkFloat = 0;
    entry->vkIsAvailable = 0;
    entry->qj = -1;
    entry->qk = -1;
    entry->dest = -1;
    entry->addr = -1;

    return entry;
}

// initialize the reservation station status table
void initResStationStatusTable(ResStationStatusTable *resStationTable) {

    // declare the number of res stations for each functional unit
    resStationTable->numIntStations = 4;
    resStationTable->numLoadStations = 2;
    resStationTable->numStoreStations = 2;
    resStationTable->numFPAddStations = 3;
    resStationTable->numFPMultStations = 3;
    resStationTable->numFPDivStations = 2;
    resStationTable->numBUStations = 2;

    // allocate arrays to store reservation station status table entries for each functional unit
    resStationTable->intEntries = malloc(resStationTable->numIntStations * sizeof(ResStationStatusTableEntry *));
    resStationTable->loadEntries = malloc(resStationTable->numLoadStations * sizeof(ResStationStatusTableEntry *));
    resStationTable->storeEntries = malloc(resStationTable->numStoreStations * sizeof(ResStationStatusTableEntry *));
    resStationTable->fpAddEntries = malloc(resStationTable->numFPAddStations * sizeof(ResStationStatusTableEntry *));
    resStationTable->fpMultEntries = malloc(resStationTable->numFPMultStations * sizeof(ResStationStatusTableEntry *));
    resStationTable->fpDivEntries = malloc(resStationTable->numFPDivStations * sizeof(ResStationStatusTableEntry *));
    resStationTable->buEntries = malloc(resStationTable->numBUStations * sizeof(ResStationStatusTableEntry *));

    // initialize INT functional unit reservation station entries
    for (int i = 0; i < resStationTable->numIntStations; i++) {
        ResStationStatusTableEntry *entry = newResStationStatusTableEntry();
        
        entry->fuType = FUType_INT;
        entry->resStationIndex = i;

        resStationTable->intEntries[i] = entry;
    }

    // initialize load functional unit reservation station entries
    for (int i = 0; i < resStationTable->numLoadStations; i++) {
        ResStationStatusTableEntry *entry = newResStationStatusTableEntry();
        
        entry->fuType = FUType_LOAD;
        entry->resStationIndex = i;

        resStationTable->loadEntries[i] = entry;
    }

    // initialize store functional unit reservation station entries
    for (int i = 0; i < resStationTable->numStoreStations; i++) {
        ResStationStatusTableEntry *entry = newResStationStatusTableEntry();
        
        entry->fuType = FUType_STORE;
        entry->resStationIndex = i;

        resStationTable->storeEntries[i] = entry;
    }

    // initialize FPadd functional unit reservation station entries
    for (int i = 0; i < resStationTable->numFPAddStations; i++) {
        ResStationStatusTableEntry *entry = newResStationStatusTableEntry();
        
        entry->fuType = FUType_FPAdd;
        entry->resStationIndex = i;

        resStationTable->fpAddEntries[i] = entry;
    }

    // initialize FPMult functional unit reservation station entries
    for (int i = 0; i < resStationTable->numFPMultStations; i++) {
        ResStationStatusTableEntry *entry = newResStationStatusTableEntry();
        
        entry->fuType = FUType_FPMult;
        entry->resStationIndex = i;

        resStationTable->fpMultEntries[i] = entry;
    }

    // initialize FPdiv functional unit reservation station entries
    for (int i = 0; i < resStationTable->numFPDivStations; i++) {
        ResStationStatusTableEntry *entry = newResStationStatusTableEntry();
        
        entry->fuType = FUType_FPDiv;
        entry->resStationIndex = i;

        resStationTable->fpDivEntries[i] = entry;
    }

    // initialize BU functional unit reservation station entries
    for (int i = 0; i < resStationTable->numBUStations; i++) {
        ResStationStatusTableEntry *entry = newResStationStatusTableEntry();
        
        entry->fuType = FUType_BU;
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

    if (resStationTable->fpMultEntries) {
        for (int i = 0; i < resStationTable->numFPMultStations; i++) {
            free(resStationTable->fpMultEntries[i]);
        }
        free(resStationTable->fpMultEntries);
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
    if (fuType == FUType_INT) {
        return resStationTable->numIntStations;
    } else if (fuType == FUType_LOAD) {
        return resStationTable->numLoadStations;
    } else if (fuType == FUType_LOAD) {
        return resStationTable->numStoreStations;
    } else if (fuType == FUType_FPAdd) {
        return resStationTable->numFPAddStations;
    } else if (fuType == FUType_FPMult) {
        return resStationTable->numFPMultStations;
    } else if (fuType == FUType_FPDiv) {
        return resStationTable->numFPDivStations;
    } else if (fuType == FUType_BU) {
        return resStationTable->numBUStations;
    } else {
        printf("error: invalid FunctionalUnitType used while getting number of reservation stations...\n");
        exit(1);
    }
}

// returns the reservation station entry list for a given functional unit
ResStationStatusTableEntry **resStationEntriesForFunctionalUnit(ResStationStatusTable *resStationTable, int fuType) {
    if (fuType == FUType_INT) {
        return resStationTable->intEntries;
    } else if (fuType == FUType_LOAD) {
        return resStationTable->loadEntries;
    } else if (fuType == FUType_LOAD) {
        return resStationTable->storeEntries;
    } else if (fuType == FUType_FPAdd) {
        return resStationTable->fpAddEntries;
    } else if (fuType == FUType_FPMult) {
        return resStationTable->fpMultEntries;
    } else if (fuType == FUType_FPDiv) {
        return resStationTable->fpDivEntries;
    } else if (fuType == FUType_BU) {
        return resStationTable->buEntries;
    } else {
        printf("error: invalid FunctionalUnitType used while get reservation station entries array...\n");
        exit(1);
    }
}

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
        return resStationTable->fpMultEntries;
    } else if (instType == FDIV) {
        return resStationTable->fpDivEntries;
    } else if (instType == BNE) {
        return resStationTable->buEntries;
    } else {
        printf("got invalid instruction type while trying to get the reservation station entry array for an instruction, this should never happen...");
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

    //printf("indexForFreeResStation: ")

    enum InstructionType instType = inst->type;

    // search for a reservation station needed by the given instruction
    if (instType == ADD || instType == ADDI || instType == SLT) {
        return indexForFreeResStation(resStationTable, FUType_INT);
    } else if (instType == FLD) {
        return indexForFreeResStation(resStationTable, FUType_LOAD);
    } else if (instType == FSD) {
        return indexForFreeResStation(resStationTable, FUType_STORE);
    } else if (instType == FADD || instType == FSUB) {
        return indexForFreeResStation(resStationTable, FUType_FPAdd);
    } else if (instType == FMUL) {
        return indexForFreeResStation(resStationTable, FUType_FPMult);
    } else if (instType == FDIV) {
        return indexForFreeResStation(resStationTable, FUType_FPDiv);
    } else if (instType == BNE) {
        return indexForFreeResStation(resStationTable, FUType_BU);
    } else {
        printf("got invalid instruction type while trying to get the index of a free reservation station, this should never happen...");
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
void setResStationEntryOperandAvailability(ResStationStatusTableEntry *entry, RegisterStatusTable *regTable, RegisterFile *regFile, int sourceNum, int physReg, int resultType) {

    // get the current ROB that will write to the source physical register
    int robIndex = getRegisterStatusTableEntryVal(regTable, physReg);

    // source is not associated with a ROB entry, read from register file
    if (robIndex == -1) {
        if (sourceNum == 1) {
            if (resultType == INST_VAL_INT) {
                entry->vjInt = readRegisterFileInt(regFile, physReg);
            } else {
                entry->vjFloat = readRegisterFileFloat(regFile, physReg);
            }
            
            entry->vjIsAvailable = 1;
        } else {
            if (resultType == INST_VAL_INT) {
                entry->vkInt = readRegisterFileInt(regFile, physReg);
            } else {
                entry->vkFloat = readRegisterFileFloat(regFile, physReg);
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

            entry->op = FUOp_ADD;

            // read first operand from register file or set source ROB 
            setResStationEntryOperandAvailability(entry, regTable, regFile, 1, inst->source1PhysReg, INST_VAL_INT);

            // manually do the second operand since its value is available within the instruction itself
            entry->vkInt = inst->imm;
            entry->vkIsAvailable = 1;
        
        // add ADD or SLT instructions to the reservation station
        } else {
            
            entry->op = instType == ADD ? FUOp_ADD : FUOp_SLT;

            // read operands from register file or set source ROB 
            setResStationEntryOperandAvailability(entry, regTable, regFile, 1, inst->source1PhysReg, INST_VAL_INT);
            setResStationEntryOperandAvailability(entry, regTable, regFile, 2, inst->source2PhysReg, INST_VAL_INT);
        }

    // add instructions that need the FPAdd functional unit to the reservation station
    } else if (instType == FADD || instType == FSUB || instType == FMUL || instType == FDIV) {

        // set the entry's add or sub operation for FADD and FSUB instructions
        if (instType == FADD || instType == FSUB) {
            entry->op = instType == FADD ? FUOp_ADD : FUOp_SUB;
        }

        // read operands from register file or set source ROB 
        setResStationEntryOperandAvailability(entry, regTable, regFile, 1, inst->source1PhysReg, INST_VAL_FLOAT);
        setResStationEntryOperandAvailability(entry, regTable, regFile, 2, inst->source2PhysReg, INST_VAL_FLOAT);
    
    // add instructions that need the load functional unit to the reservation station
    } else if (instType == FLD) {

    // add instructions that need the store functional unit to the reservation station
    } else if (instType == FSD) {
    
    // add instructions that need the BU functional unit to the reservation station
    } else if (instType == BNE) {

    }    

    printf("added instruction: %p to reservation station: %i\n", inst, resStationIndex);
}

// helper method to process int forwarding for any list of reservation station status table entries
void processIntForwardingForResStationEntries(ResStationStatusTableEntry **entries, int numEntries, IntFUResult *intResult) {

    for (int i = 0; i < numEntries; i++) {
        ResStationStatusTableEntry *entry = entries[i];

        // only update the entry's operands if they are currently avaialble and the result ROB matches the source ROB
        if (!entry->vjIsAvailable && entry->qj == intResult->destROB) {
            printf("reservation station index: %i received forwarded int: %i for vj\n", entry->resStationIndex, intResult->result);

            entry->vjIsAvailable = 1;
            entry->vjInt = intResult->result;
            entry->qj = -1;
        }

        if (!entry->vkIsAvailable && entry->qk == intResult->destROB) {
            printf("reservation station index: %i received forwarded int: %i for vk\n", entry->resStationIndex, intResult->result);

            entry->vkIsAvailable = 1;
            entry->vkInt = intResult->result;
            entry->qk = -1;
        }
    }
}

// helper method to process float forwarding for any list of reservation station status table entries
void processFloatForwardingForResStationEntries(ResStationStatusTableEntry **entries, int numEntries, FloatFUResult *floatResult) {

    for (int i = 0; i < numEntries; i++) {
        ResStationStatusTableEntry *entry = entries[i];

        // only update the entry's operands if they are currently avaialble and the result ROB matches the source ROB
        if (!entry->vjIsAvailable && entry->qj == floatResult->destROB) {
            printf("reservation station index: %i received forwarded float: %f for vj\n", entry->resStationIndex, floatResult->result);

            entry->vjIsAvailable = 1;
            entry->vjFloat = floatResult->result;
            entry->qj = -1;
        }

        if (!entry->vkIsAvailable && entry->qk == floatResult->destROB) {
            printf("reservation station index: %i received forwarded float: %f for vk\n", entry->resStationIndex, floatResult->result);

            entry->vkIsAvailable = 1;
            entry->vkFloat = floatResult->result;
            entry->qk = -1;
        }
    }
}

// updates the operands of reservation stations via a functional unit forwarding the int result
void forwardIntResultToResStationStatusTable(ResStationStatusTable *resStationTable, IntFUResult *intResult) {

    int robIndex = intResult->destROB;
    int resultVal = intResult->result;
    printf("forwarding int result: %i robIndex: %i to reservation stations\n", resultVal, robIndex);

    // forward int result to reservation stations that can use int registers
    processIntForwardingForResStationEntries(resStationTable->intEntries, resStationTable->numIntStations, intResult);
    processIntForwardingForResStationEntries(resStationTable->loadEntries, resStationTable->numLoadStations, intResult);
    processIntForwardingForResStationEntries(resStationTable->storeEntries, resStationTable->numStoreStations, intResult);
    processIntForwardingForResStationEntries(resStationTable->buEntries, resStationTable->numBUStations, intResult);
}

// updates the operands of reservation stations via a functional unit forwarding the float result
void forwardFloatResultToResStationStatusTable(ResStationStatusTable *resStationTable, FloatFUResult *floatResult) {

    int robIndex = floatResult->destROB;
    float resultVal = floatResult->result;
    printf("forwarding float result: %f robIndex: %i to reservation stations\n", resultVal, robIndex);

    // forward int result to reservation stations that can use int registers
    processFloatForwardingForResStationEntries(resStationTable->loadEntries, resStationTable->numLoadStations, floatResult);
    processFloatForwardingForResStationEntries(resStationTable->storeEntries, resStationTable->numStoreStations, floatResult);
    processFloatForwardingForResStationEntries(resStationTable->buEntries, resStationTable->numBUStations, floatResult);
    processFloatForwardingForResStationEntries(resStationTable->fpAddEntries, resStationTable->numFPAddStations, floatResult);
    processFloatForwardingForResStationEntries(resStationTable->fpMultEntries, resStationTable->numFPMultStations, floatResult);
    processFloatForwardingForResStationEntries(resStationTable->fpDivEntries, resStationTable->numFPDivStations, floatResult);
}

// prints the contents of the reservation station status table
void printResStationStatusTable(ResStationStatusTable *resStationTable) {

    printf("reservation station status table:\n");

    printf("INT FU reservation stations:\n");
    for (int i = 0; i < resStationTable->numIntStations; i++) {
        ResStationStatusTableEntry *entry = resStationTable->intEntries[i];
        
        printf("\tindex: %i, busy: %i, op: %s, vjInt: %i, vkInt: %i, vjIsAvail: %i, vkIsAvail: %i, qj: %i, qk: %i, dest: %i\n", 
            entry->resStationIndex, entry->busy, fuOpToString(entry->op), entry->vjInt, entry->vkInt,
            entry->vjIsAvailable, entry->vkIsAvailable, entry->qj, entry->qk, entry->dest);
    }
}