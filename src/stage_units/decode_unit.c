
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "decode_unit.h"
#include "../types/types.h"

// initialize a decode unit struct
void initDecodeUnit(DecodeUnit *decodeUnit, int NF, int NI, 
    Instruction **instFetchBuffer, int *instFetchBufferSize, int *numInstsInBuffer, 
    Instruction **instDecodeQueue, int *numInstsInQueue) {

    decodeUnit->NI = NI;

    // set fetch buffer variables
    decodeUnit->instFetchBuffer = instFetchBuffer;
    decodeUnit->instFetchBufferSize = instFetchBufferSize;
    decodeUnit->numInstsInBuffer = numInstsInBuffer;

    // set decode queue variables
    decodeUnit->instDecodeQueue = instDecodeQueue;
    decodeUnit->numInstsInQueue = numInstsInQueue;

    // initialize map table
    decodeUnit->registerMapTable = calloc(RN_SIZE, sizeof(RegisterMappingNode *)); // includes $0 and PC, remove them?

    // initialize free list
    RegisterMappingNode *prevNode = NULL;
    for (int i = 0; i < PHYS_RN_SIZE; i++) {

        RegisterMappingNode *node = malloc(sizeof(RegisterMappingNode));
        node->reg = i;
        node->next = NULL;

        if (!prevNode) {
            decodeUnit->registerFreeList = node;
        } else {
            prevNode->next = node;
        }

        prevNode = node;
    }
}

// free any elements in the decode unit stored on the heap
void teardownDecodeUnit(DecodeUnit *decodeUnit) {

    RegisterMappingNode *cur = decodeUnit->registerFreeList;
    RegisterMappingNode *prev = NULL;
    while (cur) {
        prev = cur;
        cur = cur->next;
        free(prev);
    }

    if (decodeUnit->registerMapTable) {
        for (int i = 0; i < RN_SIZE; i++) {
            cur = decodeUnit->registerMapTable[i];
            prev = NULL;
            
            while (cur) {
                prev = cur;
                cur = cur->next;
                free(prev);
            }        
        }   

        free(decodeUnit->registerMapTable);
    }
}

// adds an instruction to the output queue
void addInstructionToDecodeQueue(DecodeUnit *decodeUnit, Instruction *inst) {
    decodeUnit->instDecodeQueue[(*decodeUnit->numInstsInQueue)++] = inst;
    printf("added instruction: %p to decode queue, numInstsInQueue: %i\n", inst, *decodeUnit->numInstsInQueue);
}

// prints the current state of the map table
void printMapTable(DecodeUnit *decodeUnit) {

    printf("map table:\n");

    for (int i = 0; i < RN_SIZE; i++) {
        RegisterMappingNode *curNode = decodeUnit->registerMapTable[i];

        if (curNode) {

            printf("\t%s: ", registerEnumToString(i));
            
            while (curNode->next) {
                printf("%s -> ", physicalRegisterEnumToString(curNode->reg));
                curNode = curNode->next;
            }

            printf("%s\n", physicalRegisterEnumToString(curNode->reg));
        }
    }
}

// prints the current state of the free list
void printFreeList(DecodeUnit *decodeUnit) {

    printf("free list: ");

    RegisterMappingNode *curNode = decodeUnit->registerFreeList;

    if (curNode) {
        while (curNode->next) {
            printf("%s -> ", physicalRegisterEnumToString(curNode->reg));
            curNode = curNode->next;
        }

        printf("%s\n", physicalRegisterEnumToString(curNode->reg));
    } else {
        printf("none\n");
    }
}

// returns the next avaialble physical register in the free list
RegisterMappingNode *getFreePhysicalRegister(DecodeUnit *decodeUnit) {

    RegisterMappingNode *freePhysReg = decodeUnit->registerFreeList;
    if (freePhysReg) {
        printf("got free register: %i\n", freePhysReg->reg);
        decodeUnit->registerFreeList = freePhysReg->next;
        return freePhysReg;

    } else {
        printf("error: could not allocate physical register\n");
        return NULL;
    }
}

// adds a free physical register node to the map table for a given register
void addPhysicalRegisterToMapTable(DecodeUnit *decodeUnit, RegisterMappingNode *physRegNode, enum RegisterName reg) {
    RegisterMappingNode *mapHead = decodeUnit->registerMapTable[reg];
    physRegNode->next = mapHead;
    decodeUnit->registerMapTable[reg] = physRegNode;
}

// gets the physical register mapped to a given architecture register
enum PhysicalRegisterName readMapTable(DecodeUnit *decodeUnit, enum RegisterName reg) {

    if (reg == PC) {
        return PHYS_PC;
    } else if (reg == ZERO) {
        return PHYS_ZERO;
    } else {
        RegisterMappingNode *physRegNode = decodeUnit->registerMapTable[reg];

        if (physRegNode) {
            return physRegNode->reg;
        } else {
            return NONEPR;
        }
    }
}

// rename RISC-V registers to physical registers for a given instruction
void performRegisterRenaming(DecodeUnit *decodeUnit, Instruction *inst) {

    // rename registers for each type of instruction
    if (inst->type == FLD) {

        // always get new physical register for load destination
        RegisterMappingNode *destFreeNode = getFreePhysicalRegister(decodeUnit);
        addPhysicalRegisterToMapTable(decodeUnit, destFreeNode, inst->destReg);
        inst->destPhysReg = destFreeNode->reg;

        // get source register mapping
        enum PhysicalRegisterName source1PhysReg = readMapTable(decodeUnit, inst->source1Reg);

        // check if source register doesn't have a mapping
        if (source1PhysReg == NONEPR) {
            RegisterMappingNode *freeNode = getFreePhysicalRegister(decodeUnit);
            addPhysicalRegisterToMapTable(decodeUnit, freeNode, inst->source1Reg);
            source1PhysReg = freeNode->reg;
        }
        inst->source1PhysReg = source1PhysReg;

    } else if (inst->type == FSD) {

        // get register mappings
        enum PhysicalRegisterName destPhysReg = readMapTable(decodeUnit, inst->destReg);
        enum PhysicalRegisterName source1PhysReg = readMapTable(decodeUnit, inst->source1Reg);

        // check if destination register doesn't have a mapping
        if (destPhysReg == NONEPR) {
            RegisterMappingNode *freeNode = getFreePhysicalRegister(decodeUnit);
            addPhysicalRegisterToMapTable(decodeUnit, freeNode, inst->destReg);
            destPhysReg = freeNode->reg;
        }
        inst->destPhysReg = destPhysReg;
         
        // check if source register doesn't have a mapping
        if (source1PhysReg == NONEPR) {
            RegisterMappingNode *freeNode = getFreePhysicalRegister(decodeUnit);
            addPhysicalRegisterToMapTable(decodeUnit, freeNode, inst->source1Reg);
            source1PhysReg = freeNode->reg;
        }
        inst->source1PhysReg = source1PhysReg;

    } else if (inst->type == ADDI) {

        // always get new physical register for addi destination
        RegisterMappingNode *destFreeNode = getFreePhysicalRegister(decodeUnit);
        addPhysicalRegisterToMapTable(decodeUnit, destFreeNode, inst->destReg);
        inst->destPhysReg = destFreeNode->reg;

        // get source register mapping
        enum PhysicalRegisterName source1PhysReg = readMapTable(decodeUnit, inst->source1Reg);

        // check if source1 register doesn't have a mapping
        if (source1PhysReg == NONEPR) {
            RegisterMappingNode *freeNode = getFreePhysicalRegister(decodeUnit);
            addPhysicalRegisterToMapTable(decodeUnit, freeNode, inst->source1Reg);
            source1PhysReg = freeNode->reg;
        }
        inst->source1PhysReg = source1PhysReg;

    } else if (inst->type == ADD || inst->type == SLT || inst->type == FADD || inst->type == FSUB || inst->type == FMUL || inst->type == FDIV) {

        // always get new physical register for these instructions' destinations
        RegisterMappingNode *destFreeNode = getFreePhysicalRegister(decodeUnit);
        addPhysicalRegisterToMapTable(decodeUnit, destFreeNode, inst->destReg);
        inst->destPhysReg = destFreeNode->reg;

        // get source register mappings
        enum PhysicalRegisterName source1PhysReg = readMapTable(decodeUnit, inst->source1Reg);
        enum PhysicalRegisterName source2PhysReg = readMapTable(decodeUnit, inst->source2Reg);

        // check if source1 register doesn't have a mapping
        if (source1PhysReg == NONEPR) {
            RegisterMappingNode *freeNode = getFreePhysicalRegister(decodeUnit);
            addPhysicalRegisterToMapTable(decodeUnit, freeNode, inst->source1Reg);
            source1PhysReg = freeNode->reg;
        }
        inst->source1PhysReg = source1PhysReg;

        // check if source2 register doesn't have a mapping
        if (source2PhysReg == NONEPR) {
            RegisterMappingNode *freeNode = getFreePhysicalRegister(decodeUnit);
            addPhysicalRegisterToMapTable(decodeUnit, freeNode, inst->source2Reg);
            source2PhysReg = freeNode->reg;
        }
        inst->source2PhysReg = source2PhysReg;

    } else if (inst->type == BNE) {
        
        // get source register mappings
        enum PhysicalRegisterName source1PhysReg = readMapTable(decodeUnit, inst->source1Reg);
        enum PhysicalRegisterName source2PhysReg = readMapTable(decodeUnit, inst->source2Reg);

        // check if source1 register doesn't have a mapping
        if (source1PhysReg == NONEPR) {
            RegisterMappingNode *freeNode = getFreePhysicalRegister(decodeUnit);
            addPhysicalRegisterToMapTable(decodeUnit, freeNode, inst->source1Reg);
            source1PhysReg = freeNode->reg;
        }
        inst->source1PhysReg = source1PhysReg;

        // check if source2 register doesn't have a mapping
        if (source2PhysReg == NONEPR) {
            RegisterMappingNode *freeNode = getFreePhysicalRegister(decodeUnit);
            addPhysicalRegisterToMapTable(decodeUnit, freeNode, inst->source2Reg);
            source2PhysReg = freeNode->reg;
        }
        inst->source2PhysReg = source2PhysReg;

    } else {
        printf("error: could not match instruction type during register renaming, this should never happen\n");
    }
}

// execute decode unit's operations during a clock cycle
void cycleDecodeUnit(DecodeUnit *decodeUnit) {

    printf("\nperforming decode unit operations...\n");

    int numInstsMovedToQueue = 0;

    printf("num insts in buffer: %i\n", *decodeUnit->numInstsInBuffer);

    // loop through all instructions in the input buffer
    for (int i = 0; i < *decodeUnit->numInstsInBuffer; i++) {
        Instruction *inst = decodeUnit->instFetchBuffer[i];

        if (!inst) {
            printf("error: was unable to get instruction from decode input buffer when expected\n");
            exit(1);
        }

        // get physical registers for instruction
        performRegisterRenaming(decodeUnit, inst);

        // attempt to add instruction to the decode queue
        if (*decodeUnit->numInstsInQueue < decodeUnit->NI) {
            addInstructionToDecodeQueue(decodeUnit, inst);
            numInstsMovedToQueue++;
        } else {
            // exit loop if queue is full
            break;
        }
    }

    // remove instructions from fetch buffer that were moved to the decode queue
    if (numInstsMovedToQueue > 0) {
        for (int i = numInstsMovedToQueue; i < *decodeUnit->numInstsInBuffer; i++) {
            decodeUnit->instFetchBuffer[i - numInstsMovedToQueue] = decodeUnit->instFetchBuffer[i];
            decodeUnit->instFetchBuffer[i] = NULL;
        }
        *decodeUnit->numInstsInBuffer -= numInstsMovedToQueue;
    }
    
    printf("num instructions moved from fetch buffer to decode queue: %i\n", numInstsMovedToQueue);
}