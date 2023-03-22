
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cpu.h"
#include "types/types.h"
#include "memory/memory.h"

// creates a new CPU struct, initializes its data structures and components, and returns it
void initCPU(CPU *cpu, Params *params) {
    cpu->params = params;
   
    cpu->dataCache = malloc(sizeof(DataCache));
    initDataCache(cpu->dataCache);

    cpu->instCache = malloc(sizeof(InstCache));
    initInstCache(cpu->instCache);

    cpu->registerFile = malloc(sizeof(RegisterFile));
    initRegisterFile(cpu->registerFile);

    cpu->registerMapTable = calloc(RN_SIZE, sizeof(RegisterMappingNode *)); // includes $0 and PC, remove them?

    RegisterMappingNode *prevNode = NULL;
    for (int i = 0; i < PHYS_RN_SIZE; i++) {

        RegisterMappingNode *node = malloc(sizeof(RegisterMappingNode));
        node->reg = i;
        node->next = NULL;

        if (!prevNode) {
            cpu->registerFreeList = node;
        } else {
            prevNode->next = node;
        }
    }
}

void teardownCPU(CPU *cpu) {
    
}



// void run(CPU *cpu) {

//     for (;;) {

//         cycle_fetch_unit();
//         cycle_decode_unit();
//         cycle_issue_unit();
        

//     }
// }