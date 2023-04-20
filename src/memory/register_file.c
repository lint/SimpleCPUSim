
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../types/types.h"

#include "register_file.h"

// initialize the register file struct
void initRegisterFile(RegisterFile *registerFile) {

    registerFile->pc = 0;
    registerFile->numPhysicalRegisters = PHYS_REG_SIZE;
    registerFile->physicalRegisters = malloc(registerFile->numPhysicalRegisters * sizeof(RegisterFileEntry *));
    for (int i = 0; i < registerFile->numPhysicalRegisters; i++) {
        RegisterFileEntry *entry = malloc(sizeof(RegisterFileEntry));
        entry->floatVal = 0;
        entry->intVal = 0;
        entry->valueType = VALUE_TYPE_NONE;

        registerFile->physicalRegisters[i] = entry;
        
    }
}

// free any register file elements that are stored on the heap
void teardownRegisterFile(RegisterFile *registerFile) {
    
    if (registerFile->physicalRegisters) {
        for (int i = 0; i < registerFile->numPhysicalRegisters; i++) {
            free(registerFile->physicalRegisters[i]);
        }
        free(registerFile->physicalRegisters);
    }
}

// read an integer value from the register file
int readRegisterFileInt(RegisterFile *registerFile, enum PhysicalRegisterName reg) {

    // check if reading the value of PC
    if (reg == PHYS_REG_PC) {
        return registerFile->pc;
    // check if reading the value of $0
    } else if (reg == PHYS_REG_ZERO) {
        return 0;
    // error if invalid register
    } else if (reg < 0 || reg >= registerFile->numPhysicalRegisters) {
        printf("error: tried to read integer register: %i which is not in the register file\n", reg);
        exit(1);
    }

    // return the value of the register
    RegisterFileEntry *entry = registerFile->physicalRegisters[reg];
    entry->valueType = VALUE_TYPE_INT;

    printf("read register: %s, int: %i\n", physicalRegisterNameToString(reg), entry->intVal);

    return entry->intVal;
}

// read a float value from the register file
float readRegisterFileFloat(RegisterFile *registerFile, enum PhysicalRegisterName reg) {
    
    // return the value of $0 as a float
    if (reg == PHYS_REG_ZERO) {
        return 0;
    // error if invalid register
    } else if (reg < 0 || reg >= registerFile->numPhysicalRegisters) {
        printf("error: tried to read float register: %i which is not in the register file\n", reg);
        exit(1);
    }

    // return the value of the register
    RegisterFileEntry *entry = registerFile->physicalRegisters[reg];
    entry->valueType = VALUE_TYPE_FLOAT;

    printf("read register: %s, float: %f\n", physicalRegisterNameToString(reg), entry->floatVal);
    
    return entry->floatVal;
}

// writes an integer to the register file
void writeRegisterFileInt(RegisterFile *registerFile, enum PhysicalRegisterName reg, int value){
    
    // check if writing to PC
    if (reg == PHYS_REG_PC) {
        registerFile->pc = value;
        return;
    // check if attempting to write to the $0 register
    } else if (reg == PHYS_REG_ZERO) {
        printf("error: attempted to write to register $0 which cannot be done\n");
        exit(1);
    // error if invalid register
    } else if (reg < 0 || reg >= registerFile->numPhysicalRegisters) {
        printf("error: tried to write integer to register: %i which is not in the register file\n", reg);
        exit(1);
    }

    // write the value to the register
    RegisterFileEntry *entry = registerFile->physicalRegisters[reg];
    entry->intVal = value;
    entry->valueType = VALUE_TYPE_INT;
} 

// write a float to the register file
void writeRegisterFileFloat(RegisterFile *registerFile, enum PhysicalRegisterName reg, float value) {
    
    // check if writing to PC
    if (reg == PHYS_REG_PC) {
        printf("error: attempted to write a float to register PC which cannot be done\n");
        exit(1);
    // check if attempting to write to the $0 register
    } else if (reg == PHYS_REG_ZERO) {
        printf("error: attempted to write to register $0 which cannot be done\n");
        exit(1);
    // error if invalid register
    } else if (reg < 0 || reg >= registerFile->numPhysicalRegisters) {
        printf("error: tried to write float register: %i which is not in the register file\n", reg);
        exit(1);
    }

    // write the value to the register
    RegisterFileEntry *entry = registerFile->physicalRegisters[reg];
    entry->floatVal = value;
    entry->valueType = VALUE_TYPE_FLOAT;
} 

// print the contents of the register file
void printRegisterFile(RegisterFile *registerFile) {

    printf("register file:\n");

    printf("\tPC: %i\n", readRegisterFileInt(registerFile, PHYS_REG_PC));

    // loop over every physical register 
    for (int i = 0; i < registerFile->numPhysicalRegisters; i++) {
        RegisterFileEntry *entry = registerFile->physicalRegisters[i];

        if (entry->valueType == VALUE_TYPE_INT) {
            printf("\t%s: %i\n", physicalRegisterNameToString(i), entry->intVal);
        } else if (entry->valueType == VALUE_TYPE_FLOAT) {
            printf("\t%s: %f\n", physicalRegisterNameToString(i), entry->floatVal);
        } else {
            printf("\t%s: 0 (never used)\n", physicalRegisterNameToString(i));
        }
    }
}