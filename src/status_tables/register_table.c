
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../types/types.h"
#include "register_table.h"

// initialize the register status table
void initRegisterStatusTable(RegisterStatusTable *regTable) {
    
    regTable->numEntries = PHYS_RN_SIZE;
    regTable->entries = malloc(regTable->numEntries * sizeof(int));

    for (int i = 0; i < regTable->numEntries; i++) {
        regTable->entries[i] = -1;
    }
}

// free any data elements of the register status table struct that are stored on the heap
void teardownRegisterStatusTable(RegisterStatusTable *regTable) {
    if (regTable->entries) {
        free (regTable->entries);
    }
}

// sets the ROB index for a given register
void setRegisterStatusTableEntryVal(RegisterStatusTable *regTable, enum PhysicalRegisterName reg, int robIndex) {
    printf("set register status table entry: %i to ROB index: %i\n", reg, robIndex);
    regTable->entries[reg] = robIndex;
}

// sets the ROB index to -1 for a given register
void resetRegisterStatusTableEntryVal(RegisterStatusTable *regTable, enum PhysicalRegisterName reg) {
    setRegisterStatusTableEntryVal(regTable, reg, -1);
}

// gets the ROB index for a given register
int getRegisterStatusTableEntryVal(RegisterStatusTable *regTable, enum PhysicalRegisterName reg) {
    return regTable->entries[reg];
}

// prints the contents of the register status table
void printRegisterStatusTable(RegisterStatusTable *regTable) {

    printf("register status table:\n");

    for (int i = 0; i < regTable->numEntries; i++) {
        if (regTable->entries[i] != -1) {
            printf("\treg: %s, robIndex: %i\n", physicalRegisterEnumToString(i), regTable->entries[i]);
        }
    }
}