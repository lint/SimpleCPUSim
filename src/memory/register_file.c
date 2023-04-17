
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "register_file.h"
#include "../types/types.h"

// intializes a struct representing a register file
void initRegisterFile(RegisterFile *registerFile) {
    printf("initalizing register file...\n");

    registerFile->pc = 0;
    registerFile->numEntries = PHYS_RN_SIZE;
    registerFile->entries = malloc(registerFile->numEntries * sizeof(RegisterFileEntry *));
    for (int i = 0; i < registerFile->numEntries; i++) {
        RegisterFileEntry *entry = malloc(sizeof(RegisterFileEntry));
        entry->floatVal = 0;
        entry->intVal = 0;
        entry->valType = INST_VAL_INT;
        registerFile->entries[i] = entry;
    }
}

// frees elements of the register file that are stored on the heap
void teardownRegisterFile(RegisterFile *registerFile) {
    if (registerFile->entries) {
        for (int i = 0; i < registerFile->numEntries; i++) {
            free(registerFile->entries[i]);
        }
        free(registerFile->entries);
    }
}

// returns the value of a given physical register
int readRegisterFileInt(RegisterFile *registerFile, enum PhysicalRegisterName reg) {
    
    // error if invalid register
    if (reg < 0 || (reg >= PHYS_RN_SIZE && reg != PHYS_PC && reg != PHYS_ZERO)) {
        printf("error: tried to read register: %i which is not in the register file\n", reg);
        return 0;
    }

    // get either PC or ZERO special registers
    if (reg == PHYS_PC) {
        return registerFile->pc;
    } else if (reg == PHYS_ZERO) {
        return 0;
    }

    // read register file for physical registers
    return registerFile->entries[reg]->intVal;
}

// returns the value of a given physical register 
float readRegisterFileFloat(RegisterFile *registerFile, enum PhysicalRegisterName reg) {
    
    // error if invalid register
    if (reg < 0 || (reg >= PHYS_RN_SIZE && reg != PHYS_PC && reg != PHYS_ZERO)) {
        printf("error: tried to read register: %i which is not in the register file\n", reg);
        return 0;
    }

    // get either PC or ZERO special registers
    if (reg == PHYS_PC) {
        return (float)registerFile->pc;
    } else if (reg == PHYS_ZERO) {
        return 0;
    }

    // read register file for physical registers
    return registerFile->entries[reg]->floatVal;
}

// writes a value to a given physical register
void writeRegisterFileInt(RegisterFile *registerFile, enum PhysicalRegisterName reg, int value) {
    
    // error if invalid register
    if (reg < 0 || (reg >= PHYS_RN_SIZE && reg != PHYS_PC && reg != PHYS_ZERO)) {
        printf("error: tried to write to register: %i which is not in the register file\n", reg);
        return;
    }

    // get either PC or ZERO special registers
    if (reg == PHYS_PC) {
        registerFile->pc = value;
        return;
    } else if (reg == PHYS_ZERO) {
        printf("error: tried to write to $0 register\n");
        return;
    }

    // write to the physical register
    RegisterFileEntry *entry = registerFile->entries[reg];
    entry->intVal = value;
    entry->floatVal = (float)value;
    entry->valType = INST_VAL_INT;
}

// writes a value to a given physical register
void writeRegisterFileFloat(RegisterFile *registerFile, enum PhysicalRegisterName reg, float value) {
    
    // error if invalid register
    if (reg < 0 || (reg >= PHYS_RN_SIZE && reg != PHYS_PC && reg != PHYS_ZERO)) {
        printf("error: tried to write to register: %i which is not in the register file\n", reg);
        return;
    }

    // get either PC or ZERO special registers
    if (reg == PHYS_PC) {
        printf("error: tried to write a float value to the PC register\n");
        return;
    } else if (reg == PHYS_ZERO) {
        printf("error: tried to write to $0 register\n");
        return;
    }

    // write to the physical register
    RegisterFileEntry *entry = registerFile->entries[reg];
    entry->intVal = (int)value;
    entry->floatVal = value;
    entry->valType = INST_VAL_FLOAT;
}

// prints the current contents of the register file
void printRegisterFile(RegisterFile *registerFile) {

}