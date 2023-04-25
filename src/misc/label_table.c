
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "label_table.h"

// intialize an instance of a label table
void initLabelTable(LabelTable *labelTable) {
    labelTable->labelTableHead = NULL;
}

// free any label table elements that are stored on the heap
void teardownLabelTable(LabelTable *labelTable) {

    LabelTableEntry *curr = labelTable->labelTableHead;
    LabelTableEntry *prev = NULL;

    while (curr) {
        prev = curr;
        curr = curr->next;
        free(prev->label);
        free(prev);
    }
}

// adds a new entry for a label and address
void addEntryToLabelTable(LabelTable *labelTable, int addr, char *label) {
    
    // create new label table entry
    LabelTableEntry *entry = malloc(sizeof(LabelTableEntry));
    entry->addr = addr;
    entry->label = label;

    // add entry to the head of the list
    entry->next = labelTable->labelTableHead;
    labelTable->labelTableHead = entry;
}

// gets the address of the instruction associated with a given label
int getAddressForLabel(LabelTable *labelTable, char *label) {

    LabelTableEntry *curr = labelTable->labelTableHead;

    while (curr) {
        if (!strcmp(label, curr->label)) {
            return curr->addr;
        }
        curr = curr->next;
    }

    return -1;
}

// prints the contents of the label table
void printLabelTable(LabelTable *labelTable) {

    printf("label table: \n");

    LabelTableEntry *curr = labelTable->labelTableHead;

    while (curr) {
        printf("\tlabel: %s, addr: %i\n", curr->label, curr->addr);
        curr = curr->next;
    }
}