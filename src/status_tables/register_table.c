
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../misc/misc.h"
#include "register_table.h"

// initialize the register status table
void initRegisterStatusTable(RegisterStatusTable *regTable) {
    regTable->headEntry = NULL;
}

// free any data elements of the register status table struct that are stored on the heap
void teardownRegisterStatusTable(RegisterStatusTable *regTable) {

    RegisterStatusTableEntry *curr = regTable->headEntry;
    RegisterStatusTableEntry *prev = NULL;
    
    while (curr) {
        prev = curr;
        curr = curr->next;
        free(prev);
    }       
}

// gets the entry in the register status table for a given register
RegisterStatusTableEntry *registerStatusTableEntryForReg(RegisterStatusTable *regTable, ArchRegister *reg) {

    RegisterStatusTableEntry *curr = regTable->headEntry;

    if (!curr) {
        return NULL;
    }

    // loop through all entries in the register status table
    while (curr) { 

        // check if the current register status table entry matches the given register details
        if (archRegistersAreEqual(reg, curr->reg)) {
            return curr;
        }

        curr = curr->next;
    }

    // matching entry not found, return null
    return NULL;
}

// gets the ROB index for a given register
int getRegisterStatusTableEntryROBIndex(RegisterStatusTable *regTable, ArchRegister *reg) {
    RegisterStatusTableEntry *entry = registerStatusTableEntryForReg(regTable, reg);

    if (entry) {
        return entry->robIndex;
    } else {
        return -1;
    }
}

// sets the ROB index for a given register
void setRegisterStatusTableEntryROBIndex(RegisterStatusTable *regTable, ArchRegister *reg, int robIndex) {
    printf("set register status table int entry: %s to ROB index: %i\n", reg->name, robIndex);

    RegisterStatusTableEntry *entry = registerStatusTableEntryForReg(regTable, reg);

    // check if the entry exists, and create a new entry if not
    if (entry) {
        entry->robIndex = robIndex;
    } else {
        printf("\tcreating new register status table entry\n");
        entry = malloc(sizeof(RegisterStatusTableEntry));
        entry->next = regTable->headEntry;
        entry->reg = reg;
        entry->robIndex = robIndex;
        regTable->headEntry = entry;
    }
}

// prints the contents of the register status table
void printRegisterStatusTable(RegisterStatusTable *regTable) {

    printf("register status table:\n");

    RegisterStatusTableEntry *curr = regTable->headEntry;

    if (curr) {
        while (curr) {
            printf("\t%s: %i\n", curr->reg->name, curr->robIndex);
            curr = curr->next;
        }
    } else {
        printf("\tregister status table has no entries\n");
    }
}