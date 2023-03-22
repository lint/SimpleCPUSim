
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "register_file.h"
#include "../types/types.h"

// intializes a struct representing a register file
void initRegisterFile(RegisterFile *registerFile) {
    printf("initalizing register file...\n");
    registerFile->data = calloc(PHYS_RN_SIZE, sizeof(int));
    registerFile->pc = 0;
}

// frees elements of the register file that are stored on the heap
void teardownRegisterFile(RegisterFile *registerFile) {
    if (registerFile->data) {
        free(registerFile->data);
    }
}

// returns the value of a given physical register
int readRegisterFile(RegisterFile *registerFile, enum PhysicalRegisterName reg) {
    
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
    return registerFile->data[reg];
}

// writes a value to a given physical register
void writeRegisterFile(RegisterFile *registerFile, enum PhysicalRegisterName reg, int value) {
    
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
    registerFile->data[reg] = value;
}

// prints the current contents of the register file
void printRegisterFile(RegisterFile *registerFile) {

}