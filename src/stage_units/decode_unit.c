
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "decode_unit.h"
#include "../misc/misc.h"
#include "../cpu.h"
#include "../status_tables/status_tables.h"

// initialize a decode unit struct
void initDecodeUnit(DecodeUnit *decodeUnit, int NI, int NW, Instruction **fetchBuffer, int *fetchBufferSize, int *numInstsInBuffer) {

    decodeUnit->NI = NI;
    decodeUnit->NW = NW;
    decodeUnit->fetchBuffer = fetchBuffer;
    decodeUnit->fetchBufferSize = fetchBufferSize;
    decodeUnit->numInstsInBuffer = numInstsInBuffer;

    // initialize decode queue
    decodeUnit->decodeQueue = calloc(decodeUnit->NI, sizeof(Instruction *));
    decodeUnit->numInstsInQueue = 0;

    // initialize map table
    // decodeUnit->intRegMapTable = calloc(INT_REG_SIZE, sizeof(RegisterMappingNode *));
    // decodeUnit->floatRegMapTable = calloc(FLOAT_REG_SIZE, sizeof(RegisterMappingNode *));

    decodeUnit->mapTableHead = NULL;

    // initialize free list
    RegisterMappingNode *prevNode = NULL;
    for (int i = 0; i < PHYS_REG_SIZE; i++) {

        RegisterMappingNode *node = malloc(sizeof(RegisterMappingNode));
        node->reg = i;
        node->next = NULL;

        if (!prevNode) {
            decodeUnit->freeList = node;
        } else {
            prevNode->next = node;
        }

        prevNode = node;
    }
}

// free any elements in the decode unit stored on the heap
void teardownDecodeUnit(DecodeUnit *decodeUnit) {

    // free the free list
    RegisterMappingNode *currMappingNode = decodeUnit->freeList;
    RegisterMappingNode *prevMappingNode = NULL;
    while (currMappingNode) {
        prevMappingNode = currMappingNode;
        currMappingNode = currMappingNode->next;
        free(prevMappingNode);
    }

    // free the map table
    MapTableEntry *currMapTableEntry = decodeUnit->mapTableHead;
    MapTableEntry *prevMapTableEntry = NULL;
    while (currMapTableEntry) {

        currMappingNode = currMapTableEntry->mapHead;
        prevMappingNode = NULL;
        
        while (currMappingNode) {
            prevMappingNode = currMappingNode;
            currMappingNode = currMappingNode->next;
            free(prevMappingNode);
        }       

        prevMapTableEntry = currMapTableEntry;
        currMapTableEntry = currMapTableEntry->next;
        free(prevMapTableEntry);
    }

    // // free the integer register map table
    // if (decodeUnit->intRegMapTable) {
    //     for (int i = 0; i < INT_REG_SIZE; i++) {
    //         cur = decodeUnit->intRegMapTable[i];
    //         prev = NULL;
            
    //         while (cur) {
    //             prev = cur;
    //             cur = cur->next;
    //             free(prev);
    //         }        
    //     }   

    //     free(decodeUnit->intRegMapTable);
    // }
}

// adds an instruction to the decode queue
void addInstructionToDecodeQueue(DecodeUnit *decodeUnit, Instruction *inst) {
    decodeUnit->decodeQueue[decodeUnit->numInstsInQueue++] = inst;
    printf("added instruction: %p to decode queue, numInstsInQueue: %i\n", inst, decodeUnit->numInstsInQueue);
}

// prints the current state of the map table
void printMapTable(DecodeUnit *decodeUnit) {

    printf("map table:\n");

    MapTableEntry *currMapTableEntry = decodeUnit->mapTableHead;
    while (currMapTableEntry) {

        RegisterMappingNode *currMappingNode = currMapTableEntry->mapHead;
        
        if (currMappingNode) {

            printf("\t%s: ", currMapTableEntry->reg->name);
            
            while (currMappingNode->next) {
                printf("%s -> ", physicalRegisterNameToString(currMappingNode->reg));
                currMappingNode = currMappingNode->next;
            }

            printf("%s\n", physicalRegisterNameToString(currMappingNode->reg));
        } else {
            printf("\t%s: NONE\n", currMapTableEntry->reg->name);
        }    

        currMapTableEntry = currMapTableEntry->next;
    }

    // for (int i = 0; i < INT_REG_SIZE; i++) {
    //     RegisterMappingNode *curNode = decodeUnit->intRegMapTable[i];

    //     if (curNode) {

    //         printf("\t%s: ", intRegisterNameToString(i));
            
    //         while (curNode->next) {
    //             printf("%s -> ", renameRegisterNameToString(curNode->reg));
    //             curNode = curNode->next;
    //         }

    //         printf("%s\n", renameRegisterNameToString(curNode->reg));
    //     }
    // }
}

// prints the current state of the free list
void printFreeList(DecodeUnit *decodeUnit) {
    printf("free list: ");

    RegisterMappingNode *curNode = decodeUnit->freeList;

    if (curNode) {
        while (curNode->next) {
            printf("%s -> ", physicalRegisterNameToString(curNode->reg));
            curNode = curNode->next;
        }

        printf("%s\n", physicalRegisterNameToString(curNode->reg));
    } else {
        printf("none\n");
    }
}

// helper method to print the current state of the decoded instruction queue
void printDecodeQueue(DecodeUnit *decodeUnit) {

    printf("instruction decode queue: %p, size: %i, numInsts: %i, items: ", decodeUnit->decodeQueue, decodeUnit->NI, decodeUnit->numInstsInQueue);

    for (int i = 0; i < decodeUnit->numInstsInQueue; i++) {
        printf("%p, ", decodeUnit->decodeQueue[i]);
        //printInstruction(*decodeUnit->instDecodeQueue[i]);
    }

    printf("\n");
}

// returns the number of rename registers available in the free list
int numFreePhysicalRegistersAvailable(DecodeUnit *decodeUnit) {
    
    int numFreeRegs = 0;
    RegisterMappingNode *curNode = decodeUnit->freeList;

    if (curNode) {
        while (curNode->next) {
            curNode = curNode->next;
            numFreeRegs++;
        }
    }

    return numFreeRegs;
}

// returns the next avaialble physical register in the free list
RegisterMappingNode *getFreePhysicalRegister(DecodeUnit *decodeUnit) {
    
    // get the free rename register which is at the head of the free list
    RegisterMappingNode *freeRenameReg = decodeUnit->freeList;
    if (freeRenameReg) {
        printf("got free register: %s\n", physicalRegisterNameToString(freeRenameReg->reg));
        
        // remove the register from the free list
        decodeUnit->freeList = freeRenameReg->next;
        freeRenameReg->next = NULL;
        return freeRenameReg;

    } else {
        printf("error: could not allocate physical register\n");
        return NULL;
    }
}

// returns the map table entry associated with a given register
MapTableEntry *mapTableEntryForRegister(DecodeUnit *decodeUnit, ArchRegister *reg) {

    MapTableEntry *curr = decodeUnit->mapTableHead;

    if (!curr) {
        return NULL;
    }

    // loop through all entries in the map table
    while (curr) { 

        // check if the current map table entry matches the given register details
        if (archRegistersAreEqual(reg, curr->reg)) {
            return curr;
        }
        curr = curr->next;
    }

    return NULL;
}

// returns the current mapped physical register for a given architecture register
enum PhysicalRegisterName physicalRegisterMappingForReg(DecodeUnit *decodeUnit, ArchRegister *reg) {

    MapTableEntry *entry = mapTableEntryForRegister(decodeUnit, reg);

    if (entry && entry->mapHead) {
        return entry->mapHead->reg;
    } else {
        return PHYS_REG_NONE;
    }
}

// adds a free physical register node to the map table for a given register
void addPhysicalRegisterToMapTable(DecodeUnit *decodeUnit, RegisterMappingNode *physRegNode, ArchRegister *reg) {

    printf("add physical register: %s to map table for: %s\n", physicalRegisterNameToString(physRegNode->reg), reg->name);

    MapTableEntry *mapTableEntry = mapTableEntryForRegister(decodeUnit, reg);

    // the given register currently does not have a map table entry, add it
    if (!mapTableEntry) {
        mapTableEntry = malloc(sizeof(MapTableEntry));
        mapTableEntry->reg = reg;
        mapTableEntry->next = decodeUnit->mapTableHead;
        decodeUnit->mapTableHead = mapTableEntry;
    }

    RegisterMappingNode *regMapHead = mapTableEntry->mapHead;
    physRegNode->next = regMapHead;
    mapTableEntry->mapHead = physRegNode;
}

// gets the rename register currently mapped to a given register
enum PhysicalRegisterName readMapTableForReg(DecodeUnit *decodeUnit, ArchRegister *reg) {

    // check if trying to get mapping for PC or $0
    if (reg->regType == ARCH_REG_PC) {
        return PHYS_REG_PC;
    } else if (reg->regType == ARCH_REG_ZERO) {
        return PHYS_REG_ZERO;
    }
    
    MapTableEntry *entry = mapTableEntryForRegister(decodeUnit, reg);

    if (entry) {
        return entry->mapHead->reg;
    } else {
        return PHYS_REG_NONE;
    }
}

// returns the number of physical register 
int numPhysicalRegisterMappingsForReg(DecodeUnit *decodeUnit, ArchRegister *reg) {
    MapTableEntry *entry = mapTableEntryForRegister(decodeUnit, reg);

    if (!entry) {
        return 0;
    }

    int numMappings = 0;
    RegisterMappingNode *curr = entry->mapHead;

    while (curr) {
        numMappings++;
        curr = curr->next;
    }

    return numMappings;
}

// get the last physical register node in the free list
RegisterMappingNode *getFreeListTailNode(DecodeUnit *decodeUnit) {

    RegisterMappingNode *curr = decodeUnit->freeList;

    // return NULL if the free list is currently empty
    if (!curr) {
        return NULL;
    }

    // get the tail node
    while (curr->next) {
        curr = curr->next;
    }

    return curr;
}

// removes the oldest physical register in the map table for a given register and adds it back to the free list
void popOldPhysicalRegisterMappingForReg(DecodeUnit *decodeUnit, ArchRegister *reg) {

    MapTableEntry *entry = mapTableEntryForRegister(decodeUnit, reg);

    if (!entry) {
        printf("error: tried to pop oldest physical register mapping for reg: %s which does not have a map table entry\n", reg->name);
        return;
    }

    int numMappings = numPhysicalRegisterMappingsForReg(decodeUnit, reg);

    // do not try to pop mapping if there are no older mappings (also ensures that the first curr->next below exists)
    if (numMappings <= 1) {
        printf("error: tried to pop a mapping for reg: %s but there are no older mappings\n", reg->name);
        return;
    }

    // get the oldest mapping for the register
    RegisterMappingNode *curr = entry->mapHead;
    RegisterMappingNode *prev = NULL;
    while (curr->next) {
        prev = curr;
        curr = curr->next;
    }

    // remove the last node from the mapping list
    prev->next = NULL;

    RegisterMappingNode *freeListTail = getFreeListTailNode(decodeUnit);

    printf("adding physical register: %s to the end of the free list\n", physicalRegisterNameToString(curr->reg));

    // add the physical register node to the end of the free list
    if (freeListTail) {
        freeListTail->next = curr;
    
    // the free list is currently empty, set the popped node as the head
    } else {
        decodeUnit->freeList = curr;
    }
}

// returns the number of physical registers that need to be allocated for a given instruction
int numPhysicalRegistersNeededForInst(DecodeUnit *decodeUnit, Instruction *inst) {

    int numNeeded = 0;

    // check needed free physical allocations for each instruction type
    if (inst->type == FLD || inst->type == ADDI) {
        
        // always need to allocate a register for the destination register
        numNeeded++;

        // check if source register doesn't have a mapping
        if (physicalRegisterMappingForReg(decodeUnit, inst->source1Reg) == PHYS_REG_NONE) {
            numNeeded++;
        }

    } else if (inst->type == FSD) {

        // check if dest register doesn't have a mapping
        if (physicalRegisterMappingForReg(decodeUnit, inst->destReg) == PHYS_REG_NONE) {
            numNeeded++;
        }

        // check if source register doesn't have a mapping
        if (physicalRegisterMappingForReg(decodeUnit, inst->source1Reg) == PHYS_REG_NONE) {
            numNeeded++;
        }

    } else if (inst->type == ADD || inst->type == SLT || inst->type == FADD || inst->type == FSUB || inst->type == FMUL || inst->type == FDIV) {
        
        // always need to allocate a register for the destination register
        numNeeded++;

        // check if source1 register doesn't have a mapping
        if (physicalRegisterMappingForReg(decodeUnit, inst->source1Reg) == PHYS_REG_NONE) {
            numNeeded++;
        }

        // check if source2 register doesn't have a mapping
        if (physicalRegisterMappingForReg(decodeUnit, inst->source2Reg) == PHYS_REG_NONE) {
            numNeeded++;
        }

    } else if (inst->type == BNE) {
        
        // check if source1 register doesn't have a mapping
        if (physicalRegisterMappingForReg(decodeUnit, inst->source1Reg) == PHYS_REG_NONE) {
            numNeeded++;
        }

        // check if source2 register doesn't have a mapping
        if (physicalRegisterMappingForReg(decodeUnit, inst->source2Reg) == PHYS_REG_NONE) {
            numNeeded++;
        }
    } else {
        printf("error: could not match instruction type during register renaming, this should never happen\n");
    }

    return numNeeded;
}

// rename architectural registers to physical registers for a given instruction. returns 1 if renaming was successful, 0 if not
int performRegisterRenamingForInst(DecodeUnit *decodeUnit, Instruction *inst) {

    int numPhysRegsAvailable = numFreePhysicalRegistersAvailable(decodeUnit);
    int numNewPhysRegsNeeded = numPhysicalRegistersNeededForInst(decodeUnit, inst);

    // more physical registers need to be allocated than are currently available, renaming cannot proceed
    if (numPhysRegsAvailable < numNewPhysRegsNeeded) {
        return 0;
    }
    
    // rename registers for each type of instruction
    if (inst->type == FLD || inst->type == ADDI) {

        // get source register mapping
        enum PhysicalRegisterName source1PhysReg = readMapTableForReg(decodeUnit, inst->source1Reg);

        // check if source register doesn't have a mapping
        if (source1PhysReg == PHYS_REG_NONE) {
            RegisterMappingNode *freeNode = getFreePhysicalRegister(decodeUnit);
            addPhysicalRegisterToMapTable(decodeUnit, freeNode, inst->source1Reg);
            source1PhysReg = freeNode->reg;
        }
        inst->source1PhysReg = source1PhysReg;

        // always get new physical register for destination
        RegisterMappingNode *destFreeNode = getFreePhysicalRegister(decodeUnit);
        addPhysicalRegisterToMapTable(decodeUnit, destFreeNode, inst->destReg);
        inst->destPhysReg = destFreeNode->reg;

    } else if (inst->type == FSD) {

        // get register mappings
        enum PhysicalRegisterName destPhysReg = readMapTableForReg(decodeUnit, inst->destReg);
        enum PhysicalRegisterName source1PhysReg = readMapTableForReg(decodeUnit, inst->source1Reg);

        // check if destination register doesn't have a mapping
        if (destPhysReg == PHYS_REG_NONE) {
            RegisterMappingNode *freeNode = getFreePhysicalRegister(decodeUnit);
            addPhysicalRegisterToMapTable(decodeUnit, freeNode, inst->destReg);
            destPhysReg = freeNode->reg;
        }
        inst->destPhysReg = destPhysReg;
         
        // check if source register doesn't have a mapping
        if (source1PhysReg == PHYS_REG_NONE) {
            RegisterMappingNode *freeNode = getFreePhysicalRegister(decodeUnit);
            addPhysicalRegisterToMapTable(decodeUnit, freeNode, inst->source1Reg);
            source1PhysReg = freeNode->reg;
        }
        inst->source1PhysReg = source1PhysReg;

    } else if (inst->type == ADD || inst->type == SLT || inst->type == FADD || inst->type == FSUB || inst->type == FMUL || inst->type == FDIV) {

        // get source register mappings
        enum PhysicalRegisterName source1PhysReg = readMapTableForReg(decodeUnit, inst->source1Reg);
        enum PhysicalRegisterName source2PhysReg = readMapTableForReg(decodeUnit, inst->source2Reg);

        // check if source1 register doesn't have a mapping
        if (source1PhysReg == PHYS_REG_NONE) {
            RegisterMappingNode *freeNode = getFreePhysicalRegister(decodeUnit);
            addPhysicalRegisterToMapTable(decodeUnit, freeNode, inst->source1Reg);
            source1PhysReg = freeNode->reg;
        }
        inst->source1PhysReg = source1PhysReg;

        // check if source2 register doesn't have a mapping
        if (source2PhysReg == PHYS_REG_NONE) {
            RegisterMappingNode *freeNode = getFreePhysicalRegister(decodeUnit);
            addPhysicalRegisterToMapTable(decodeUnit, freeNode, inst->source2Reg);
            source2PhysReg = freeNode->reg;
        }
        inst->source2PhysReg = source2PhysReg;

        // always get new physical register for these instructions' destinations
        RegisterMappingNode *destFreeNode = getFreePhysicalRegister(decodeUnit);
        addPhysicalRegisterToMapTable(decodeUnit, destFreeNode, inst->destReg);
        inst->destPhysReg = destFreeNode->reg;

    } else if (inst->type == BNE) {
        
        // get source register mappings
        enum PhysicalRegisterName source1PhysReg = readMapTableForReg(decodeUnit, inst->source1Reg);
        enum PhysicalRegisterName source2PhysReg = readMapTableForReg(decodeUnit, inst->source2Reg);

        // check if source1 register doesn't have a mapping
        if (source1PhysReg == PHYS_REG_NONE) {
            RegisterMappingNode *freeNode = getFreePhysicalRegister(decodeUnit);
            addPhysicalRegisterToMapTable(decodeUnit, freeNode, inst->source1Reg);
            source1PhysReg = freeNode->reg;
        }
        inst->source1PhysReg = source1PhysReg;

        // check if source2 register doesn't have a mapping
        if (source2PhysReg == PHYS_REG_NONE) {
            RegisterMappingNode *freeNode = getFreePhysicalRegister(decodeUnit);
            addPhysicalRegisterToMapTable(decodeUnit, freeNode, inst->source2Reg);
            source2PhysReg = freeNode->reg;
        }
        inst->source2PhysReg = source2PhysReg;

    } else {
        printf("error: could not match instruction type during register renaming, this should never happen\n");
    }

    inst->regsWereRenamed = 1;

    return 1;
}

// execute decode unit's operations during a clock cycle
void cycleDecodeUnit(DecodeUnit *decodeUnit, StatusTables *statusTables, RegisterFile *regFile, StallStats *stallStats) {

    printf("\nperforming decode unit operations...\n");
    printf("num insts in buffer: %i\n", *decodeUnit->numInstsInBuffer);

    ROBStatusTable *robTable = statusTables->robTable;
    ResStationStatusTable *resStationTable = statusTables->resStationTable;
    RegisterStatusTable *regTable = statusTables->regTable;

    int numInstsMovedToQueue = 0;

    // loop through all instructions in the input buffer
    for (int i = 0; i < *decodeUnit->numInstsInBuffer; i++) {
        Instruction *inst = decodeUnit->fetchBuffer[i];

        if (!inst) {
            printf("error: was unable to get instruction from decode input buffer when expected\n");
            exit(1);
        }

        // attempt to add instruction to the decode queue
        if (decodeUnit->numInstsInQueue < decodeUnit->NI) {
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
            decodeUnit->fetchBuffer[i - numInstsMovedToQueue] = decodeUnit->fetchBuffer[i];
            decodeUnit->fetchBuffer[i] = NULL;
        }
        *decodeUnit->numInstsInBuffer -= numInstsMovedToQueue;
    }

    // check if there are any instructions in the decode queue
    if (decodeUnit->numInstsInQueue == 0) {
        printf("no instructions in decode queue\n");
        return;
    }

    printf("attempting to rename instructions...\n");

    // attempt to rename registers for instructions in the decode queue
    for (int i = 0; i < decodeUnit->numInstsInQueue; i++) {
        Instruction *inst = decodeUnit->decodeQueue[i];
        enum InstructionType instType = inst->type;

        // do not rename instructions that were previously renamed
        if (inst->regsWereRenamed) {
            continue;
        }

        // perform the renaming
        int renamingWasSuccessful = performRegisterRenamingForInst(decodeUnit, inst);


        // TODO: remove this
        // if (numFreeRenameRegistersAvailable(decodeUnit) >= 1) {

        //     RegisterMappingNode *renameRegNode = getFreeRenameRegister(decodeUnit);

        //     if (instType == ADDI || instType == ADD || instType == SLT) {

        //         addRenameRegisterToIntMapTable(decodeUnit, renameRegNode, inst->destReg);

        //     } else if (instType == FADD || instType == FSUB || instType == FMUL || instType == FDIV || instType == FLD) {

        //         addRenameRegisterToFloatMapTable(decodeUnit, renameRegNode, inst->destReg);

        //     } else if (instType == FSD) {
        //         // TODO
        //     } else if (instType == BNE ) {
        //         // TODO
        //     } else {
        //         // probably unnecessary to have at this point..
        //     }

        //     inst->renamedDestReg = renameRegNode->reg;
        //     inst->regsWereRenamed = 1;
        // }

        if (renamingWasSuccessful) {
            printf("successfully renamed inst: %p\n", inst);
        } else {
            printf("could not rename inst: %p, will again next cycle\n", inst);
            break;
        }
    }

    printMapTable(decodeUnit);

    int numInstsIssued = 0;

    printf("attempting to issue instructions...\n");

    // attempt to issue up to NW instructions 
    for (int i = 0; i < decodeUnit->NW && numInstsIssued < decodeUnit->numInstsInQueue; i++) {

        Instruction *inst = decodeUnit->decodeQueue[i];
        enum InstructionType instType = inst->type;

        // stop issuing if the registers were not renamed
        if (!inst->regsWereRenamed) {
            printf("could not issue next instruction: %p as it has not gone through register renaming yet\n", inst);
            break;
        }

        // issue instruction if free slot in ROB and reservation station is available
        if (isFreeEntryInROB(robTable) && isFreeResStationForInstruction(resStationTable, inst)) {
            numInstsIssued++;
            printInstruction(*inst);
            
            // add entry in the ROB for the given instruction and get the index it is stored at
            int robIndex = addInstToROB(robTable, inst);

            // add the values to the reservation station (just updating the reservation station table)
            addInstToResStation(resStationTable, regTable, regFile, inst, robIndex);

            // update the register status table to have the destination of the instruction
            // if (instType == ADDI || instType == ADD || instType == SLT) {
            //     setRegisterStatusTableIntEntryVal(regTable, inst->destReg, robIndex);
            // } else if (instType == FADD || instType == FSUB || instType == FMUL || instType == FDIV || instType == FLD) {
            //     setRegisterStatusTableFloatEntryVal(regTable, inst->destReg, robIndex);
            // }
            setRegisterStatusTableEntryROBIndex(regTable, inst->destReg, robIndex);
        } else {

            // check if failure to issue was caused by the ROB being full
            if (!isFreeEntryInROB(robTable)) {
                stallStats->fullROBStalls++;
                printf("encountered stall in issue unit due to ROB being full, fullROBStalls: %i\n", stallStats->fullROBStalls);
            
            // check if the failure to issue was caused by no reservation stations being available
            } else if (!isFreeResStationForInstruction(resStationTable, inst)) {
                stallStats->fullResStationStalls++;
                printf("encountered stall in issue unit due to reservation stations needed by the instruction being full, fullResStationStalls: %i\n", stallStats->fullResStationStalls);
            }

            break;
        }
    }

    // remove issued instructions from the decode queue
    if (numInstsIssued > 0) {
        printf("removing instructions from decode queue\n");

        for (int i = numInstsIssued; i < decodeUnit->numInstsInQueue; i++) {
            decodeUnit->decodeQueue[i - numInstsIssued] = decodeUnit->decodeQueue[i];
            decodeUnit->decodeQueue[i] = NULL;
        }

        decodeUnit->numInstsInQueue -= numInstsIssued;
    }
}