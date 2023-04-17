
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../types/types.h"
#include "res_station_table.h"
#include "register_table.h"
#include "../memory/memory.h"

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

// returns the index of a free INT reservation station if one is available, otherwise return -1
int indexForFreeResStation(ResStationStatusTable *resStationTable, enum FunctionalUnitType fuType) {

    int numStations = -1;
    ResStationStatusTableEntry **entries = NULL;

    // get the number of reservation stations and the entry array for the given functional unit type
    if (fuType == FUType_INT) {
        numStations = resStationTable->numIntStations;
        entries = resStationTable->intEntries;
    } else if (fuType == FUType_LOAD) {
        numStations = resStationTable->numLoadStations;
        entries = resStationTable->loadEntries;
    } else if (fuType == FUType_LOAD) {
        numStations = resStationTable->numStoreStations;
        entries = resStationTable->storeEntries;
    } else if (fuType == FUType_FPAdd) {
        numStations = resStationTable->numFPAddStations;
        entries = resStationTable->fpAddEntries;
    } else if (fuType == FUType_FPMult) {
        numStations = resStationTable->numFPMultStations;
        entries = resStationTable->fpMultEntries;
    } else if (fuType == FUType_FPDiv) {
        numStations = resStationTable->numFPDivStations;
        entries = resStationTable->fpDivEntries;
    } else if (fuType == FUType_BU) {
        numStations = resStationTable->numBUStations;
        entries = resStationTable->buEntries;
    } else {
        printf("error: invalid FunctionalUnitType used while searching for free reservation station...\n");
        exit(1);
    }
    
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

// updates the reservation station status table for a given instruction
void addInstToResStation(ResStationStatusTable *resStationTable, RegisterStatusTable *regTable, RegisterFile *regFile, Instruction *inst, int destROB) {

    enum InstructionType instType = inst->type;
    int resStationIndex = -1;

    // add ADDI instructions to the reservation station
    if (instType == ADDI) {

        resStationIndex = indexForFreeResStationForInstruction(resStationTable, inst);
        ResStationStatusTableEntry *entry = resStationTable->intEntries[resStationIndex];

        entry->op = FUOp_ADD;
        entry->dest = destROB;
        entry->busy = 1;

        // check if source1 will be located in the ROB
        int source1ROBIndex = getRegisterStatusTableEntryVal(regTable, inst->source1PhysReg);
        
        // source 1 is not associated with a ROB entry, read from register file
        if (source1ROBIndex == -1) {
            entry->vjInt = readRegisterFileInt(regFile, inst->source1PhysReg);
            entry->vjIsAvailable = 1;
        
        // source 1 is (or will be) located in the ROB
        } else {
            entry->vjIsAvailable = 0;
            entry->qj = source1ROBIndex;
        }

        entry->vkInt = inst->imm;
        entry->vkIsAvailable = 1;
    
    // add ADD instructions to the reservation station
    } else if (instType == ADD) {
        
        // resStationIndex = indexForFreeResStationForInstruction(resStationTable, inst);
        // ResStationStatusTableEntry *entry = resStationTable->intEntries[resStationIndex];

        // entry->op = FUOp_ADD;
        // entry->dest = destROB;

        // // check if sources will be located in the ROB
        // int source1ROBIndex = getRegisterStatusTableEntryVal(regTable, inst->source1PhysReg);
        // int source2ROBIndex = getRegisterStatusTableEntryVal(regTable, inst->source2PhysReg);
        
        // // source 1 is not associated with a ROB entry, read from register file
        // if (source1ROBIndex == -1) {
        //     entry->vjInt = readRegisterFileInt(regFile, inst->source1PhysReg);
        //     entry->vjIsAvailable = 1;
        
        // // source 1 is (or will be) located in the ROB
        // } else {
        //     entry->vjIsAvailable = 0;
        //     entry->qj = source1ROBIndex;
        // }
    }

    printf("added instruction: %p to reservation station: %i\n", inst, resStationIndex);
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