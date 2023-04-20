
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../types/types.h"
#include "rob_table.h"

// initialize the rob table struct
void initROBStatusTable(ROBStatusTable *robTable, int NR) {

    robTable->NR = NR;
    robTable->headEntryIndex = 0;

    robTable->entries = malloc(NR * sizeof(ROBStatusTableEntry *));

    // create empty entries in the ROB table
    for (int i = 0; i < NR; i++) {
        ROBStatusTableEntry *entry = malloc(sizeof(ROBStatusTableEntry));
        entry->index = i;
        entry->busy = 0;
        entry->inst = NULL;
        entry->state = STATE_NONE;
        entry->destReg = NULL;
        entry->renamedDestReg = PHYS_REG_NONE;
        entry->intValue = 0;
        entry->floatValue = 0;
        entry->instResultValueType = VALUE_TYPE_NONE;

        robTable->entries[i] = entry;
    }
}

// free any rob table struct elements that are stored on the heap
void teardownROBStatusTable(ROBStatusTable *robTable) {

    if (robTable->entries) {
        for (int i = 0; i < robTable->NR; i++) {
            free(robTable->entries[i]);
        }
        free(robTable->entries);
    }
}

// returns the index of the next available free slot in the ROB 
int nextFreeROBEntryIndex(ROBStatusTable *robTable) {

    for (int i = 0; i < robTable->NR; i++) {
        int index = (robTable->headEntryIndex + i) % robTable->NR;
        ROBStatusTableEntry *entry = robTable->entries[index];

        if (!entry->busy) {
            return index;
        }
    }

    return -1;
}

// returns 1 if there is an available slot in the ROB, 0 if it is full
int isFreeEntryInROB(ROBStatusTable *robTable) {
    return nextFreeROBEntryIndex(robTable) != -1;
}

// adds an instruction to the ROB and returns the ROB index that will contain the result
int addInstToROB(ROBStatusTable *robTable, Instruction *inst) {

    int robIndex = nextFreeROBEntryIndex(robTable);
    ROBStatusTableEntry *entry = robTable->entries[robIndex];

    entry->busy = 1;
    entry->inst = inst;
    entry->state = STATE_ISSUED;
    entry->destReg = inst->destReg;
    entry->renamedDestReg = inst->destPhysReg;
    entry->intValue = 0;
    entry->floatValue = 0;

    // TODO: also need to support address for store operations..............

    enum InstructionType instType = inst->type;
    if (instType == ADD || instType == ADDI || instType == SLT) {
        entry->instResultValueType = VALUE_TYPE_INT;
    } else if (instType == FLD || instType == FSD || instType == FADD || instType == FSUB || instType == FMUL || instType == FDIV) {
        entry->instResultValueType = VALUE_TYPE_FLOAT;
    } else {
        entry->instResultValueType = VALUE_TYPE_NONE;
    }

    printf("added instruction: %p to ROB entry: %i\n", inst, robIndex);

    return robIndex;
}

// prints the contents of the ROB status table
void printROBStatusTable(ROBStatusTable *robTable) {
    
    printf("ROB status table: head entry index: %i\n", robTable->headEntryIndex);

    for (int i = 0; i < robTable->NR; i++) {
        ROBStatusTableEntry *entry = robTable->entries[i];
        char *destStr = NULL;
        if (entry->destReg) {
            destStr = entry->destReg->name;
        }
        printf("\trobIndex: %i, busy: %i, inst: %p, dest: %s, renamedDest: %s, intVal: %i, floatVal: %f, ", entry->index, entry->busy, entry->inst, destStr, 
            physicalRegisterNameToString(entry->renamedDestReg), entry->intValue, entry->floatValue);
        printf("state: %s, resultType: %s\n", instStateToString(entry->state), valueTypeToString(entry->instResultValueType));
    }
}

// returns the ROB entry that is currently at the head
ROBStatusTableEntry *getHeadROBEntry(ROBStatusTable *robTable) {
    return robTable->entries[robTable->headEntryIndex];
}

// returns 1 if all robs in the ROB status table are not busy, 0 if any of them are busy
int isROBEmpty(ROBStatusTable *robTable) {

    for (int i = 0; i < robTable->NR; i++) {
        if (robTable->entries[i]->busy) {
            return 0;
        }
    } 
    
    return 1;
}