
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cpu.h"

// creates a new CPU struct, initializes its data structures and components, and returns it
void initCPU(CPU *cpu) {
    cpu->numInsts = 0;
    cpu->dataCacheSize = DATA_CACHE_INITIAL_SIZE;
    cpu->instCacheSize = INST_CACHE_INITIAL_SIZE;
    cpu->dataCache = calloc(DATA_CACHE_INITIAL_SIZE, 1);
    cpu->instCache = calloc(INST_CACHE_INITIAL_SIZE, sizeof(Instruction));
    cpu->registerFile = calloc(PHYS_RN_SIZE, sizeof(int));
}

// checks if a given address is outside the bounds of the data cache and increases its size if so
void extendDataCacheIfNeeded(CPU *cpu, int address) {
    
    if (address >= cpu->dataCacheSize) {

        int oldSize = cpu->dataCacheSize;
        unsigned char *oldCache = cpu->dataCache;

        // double size of data cache until it can fit the requested address
        int newSize = cpu->dataCacheSize * 2;
        while (newSize < address) {
            newSize *= 2;
        }

        // set the new size and reallocate the data cache
        cpu->dataCacheSize = newSize;
        cpu->dataCache = calloc(newSize, 1);
        memcpy(cpu->dataCache, oldCache, oldSize);
        free(oldCache);

        printf("address '%d' is greater than the current data cache size '%d', extending to %d bytes\n", address, oldSize, newSize);
    }
}

// doubles the size of the instruction cache
void extendInstCacheIfNeeded(CPU *cpu) {

    // only extend the instruction cache if it is currently full
    if (cpu->numInsts >= cpu->instCacheSize) {
        
        // get old values
        Instruction *oldCache = cpu->instCache;
        int oldSize = cpu->instCacheSize;
        int newSize = oldSize * 2;

        // reallocate array and copy contents
        cpu->instCacheSize = newSize;
        cpu->instCache = calloc(newSize, sizeof(Instruction *));
        memcpy(cpu->instCache, oldCache, oldSize);
        printf("extending instruction cache to %i entries\n", newSize);
    }
}

// writes a byte to a certain address in the data cache
void writeByteToDataCache(CPU *cpu, int address, unsigned char byte) {

    if (address < 0) {
        printf("error: can't write to an address less than 0\n");
        return;
    }

    // check if requested address is outside current cache size
    extendDataCacheIfNeeded(cpu, address);

    // write the byte to the cache
    cpu->dataCache[address] = byte;
}

// retrieves a byte from a certain address in the data cache
unsigned char readByteFromDataCache(CPU *cpu, int address) {

    if (address < 0) {
        printf("error: can't read from an address less than 0\n");
        return '\0';
    }

    // check if requested address is outside current cache size
    extendDataCacheIfNeeded(cpu, address);

    // read the byte from the cache and return it
    return cpu->dataCache[address];
}

// prints the contents of the data cache to the command line
void printDataCache(CPU *cpu) {

    // number of bytes to include for each row
    int entriesPerRow = 8;

    // print byte row index
    printf("%5s | ", "");
    for (int i = 0; i < entriesPerRow; i++) {
        printf("%5d", i);
    }
    printf("\n");

    // print separator
    printf("---");
    for (int i = 0; i < (entriesPerRow + 1) * 5; i++) {
        printf("-");
    }
    printf("\n");

    // print each full row of bytes
    for (int i = 0; i < cpu->dataCacheSize / entriesPerRow; i++) {
        //printf("0x%-3x", i);
        printf("%5d | ", i * entriesPerRow);
        for (int j = 0; j < entriesPerRow; j++) {
            printf("%5d", readByteFromDataCache(cpu, i * entriesPerRow + j));
        }
        printf("\n");
    }

    // print the remaining row if the cache cannot be divided evenly
    if (cpu->dataCacheSize % entriesPerRow != 0) {
        int lastRow = cpu->dataCacheSize / entriesPerRow;

        //printf("0x%-3x", i);
        printf("%5d", lastRow * entriesPerRow);
        for (int j = 0; j < cpu->dataCacheSize % entriesPerRow; j++) {
            printf("%5d", readByteFromDataCache(cpu, lastRow * entriesPerRow + j));
        }
        printf("\n");
    }
}

// returns an instruction in the instruction cache at the given address
Instruction *readInstructionCache(CPU *cpu, int address) {
    int index = address / 4; // since there are 4 bytes per instruction being simulated

    if (index < 0 || index >= cpu->instCacheSize) {
        printf("error: attempted to read instruction at address '%d' (index '%d') which is out of bounds\n", address, index);
        return NULL;
    }

    return &(cpu->instCache[index]);
}

// adds a new instruction to the instruction cache when processing the input file
void addInstructionToCache(CPU *cpu, Instruction inst) {

    // extend the instruction cache if not enough space for a new instruction
    extendInstCacheIfNeeded(cpu);

    // store the instruction
    cpu->instCache[cpu->numInsts++] = inst;
}

// give each branch instruction the target address based on the label
int resolveInstLabels(CPU *cpu) {

    // iterate through every instruction
    for (int i = 0; i < cpu->numInsts; i++) {
        Instruction *inst1 = &cpu->instCache[i];
        // check if the instruction is a branch
        if (inst1->type == BNE) {

            int foundMatch = 0;

            // check every other instruction
            for (int j = 0; j < cpu->numInsts; j++) {
                if (j == i) {
                    continue;
                }

                Instruction *inst2 = &cpu->instCache[j];

                // check if labels match
                if (!strcmp(inst1->branchTargetLabel, inst2->label)) {
                    printf("instruction: %i (address: %i) has the target label: %s matched by branch: %i\n", j, j * 4, inst2->label, i);

                    inst1->branchTargetAddr = j * 4; // simulating instructions taking 4 bytes
                    foundMatch = 1;
                    break;
                }
            }

            // return error if there is instruction that has the target label
            if (!foundMatch) {
                printf("error: branch could not find instruction with label: %s\n", inst1->branchTargetLabel);
                return 1;
            }
        }
    }

    return 0;
}

// returns the value of a given physical register
int readRegisterFile(CPU *cpu, enum PhysicalRegisterName reg) {
    
    // error if invalid register
    if (reg < 0 || reg >= PHYS_RN_SIZE) {
        printf("error: tried to read register: %i which is not in the register file\n", reg);
        return 0;
    }

    return cpu->registerFile[reg];
}

// writes a value to a given physical register
void writeRegisterFile(CPU *cpu, enum PhysicalRegisterName reg, int value) {
    
    // error if invalid register
    if (reg < 0 || reg >= PHYS_RN_SIZE) {
        printf("error: tried to write to register: %i which is not in the register file\n", reg);
        return 0;
    }

    cpu->registerFile[reg] = value;
}


// void run(CPU *cpu) {

//     for (;;) {

//         cycle_fetch_unit();
//         cycle_decode_unit();
//         cycle_issue_unit();
        

//     }
// }