
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cpu.h"
#include "types/types.h"
#include "memory/memory.h"
#include "stage_units/stage_units.h"

// creates a new CPU struct, initializes its data structures and components, and returns it
void initCPU(CPU *cpu, Params *params) {
    cpu->params = params;
   
    cpu->dataCache = malloc(sizeof(DataCache));
    initDataCache(cpu->dataCache);

    cpu->instCache = malloc(sizeof(InstCache));
    initInstCache(cpu->instCache);

    cpu->registerFile = malloc(sizeof(RegisterFile));
    initRegisterFile(cpu->registerFile);

    cpu->fetchUnit = malloc(sizeof(FetchUnit));
    initFetchUnit(cpu->fetchUnit, params->NF);

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

// free any elements of the CPU that were stored on the heap
void teardownCPU(CPU *cpu) {

    if (cpu->dataCache) {
        teardownDataCache(cpu->dataCache);
        free(cpu->dataCache);
    }

    if (cpu->instCache) {
        teardownInstCache(cpu->instCache);
        free(cpu->instCache);
    }

    if (cpu->registerFile) {
        teardownRegisterFile(cpu->registerFile);
        free(cpu->registerFile);
    }

    if (cpu->fetchUnit) {
        teardownFetchUnit(cpu->fetchUnit);
        free(cpu->fetchUnit);
    }
}

// start executing instructions on the CPU
void executeCPU(CPU *cpu) {

    int cycle = 0;

    // infinite loop to cycle the clock until execution finishes
    for (int i = 0; i < 10; i++) {

        cycleFetchUnit(cpu->fetchUnit, cpu->registerFile, cpu->instCache);

        
        // break;
        cycle++;
    }
}