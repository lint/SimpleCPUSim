
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "decode_unit.h"
#include "../misc/misc.h"
#include "../cpu.h"
#include "../status_tables/status_tables.h"

// initialize a decode unit struct
void initDecodeUnit(DecodeUnit *decodeUnit, int NI, int NW) {

    decodeUnit->NI = NI;
    decodeUnit->NW = NW;

    // initialize decode queue
    decodeUnit->decodeQueue = calloc(decodeUnit->NI, sizeof(Instruction *));
    decodeUnit->numInstsInQueue = 0;

    // initialize map table (linked list of linked lists)
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
}

// adds an instruction to the decode queue
void addInstructionToDecodeQueue(DecodeUnit *decodeUnit, LabelTable *labelTable, char *instStr, int instAddr) {

    // initialize instruction
    Instruction *inst = malloc(sizeof(Instruction));
    inst->type = INST_TYPE_NONE;
    inst->destReg = NULL;
    inst->source1Reg = NULL;
    inst->source2Reg = NULL;
    inst->destPhysReg = PHYS_REG_NONE;
    inst->source1PhysReg = PHYS_REG_NONE;
    inst->source2PhysReg = PHYS_REG_NONE;
    inst->imm = 0;
    inst->label[0] = '\0';
    inst->branchTargetLabel[0] = '\0';
    inst->regsWereRenamed = 0;
    inst->addr = instAddr;

    // copy input string to buffer so that it does not modified by strtok
    char instBuf[256];
    strcpy(instBuf, instStr);
    strcpy(inst->fullStr, instStr);

    #ifdef ENABLE_DEBUG_LOG
    printf("inst: %s\n", inst->fullStr);
    #endif

    // get the first token of the line
    char *cur = strtok(instBuf, " \n\t,");

    if (cur == NULL) {
        exit(1);
    }

    // the first token containing a colon indicates it is a label
    if (strstr(cur, ":")) {
        
        memcpy(inst->label, cur, strlen(cur)-1);
        inst->label[strlen(cur)-1] = '\0';

        cur = strtok(NULL, " \n\t,");
    }
    
    // convert the instruction type to an enum
    inst->type = stringToInstructionType(cur);

    // instruction type could not be matched
    if (inst->type == INST_TYPE_NONE) {
        
        printf("error: invalid instruction type '%s'\n", cur);
        exit(1);

    // memory access instructions
    } else if (inst->type == FLD) {

        inst->destReg = stringToArchRegister(strtok(NULL, " \n\t,"));
        inst->imm = atoi(strtok(NULL, " \n\t,()"));
        inst->source2Reg = stringToArchRegister(strtok(NULL, " \n\t,()"));

        if (!inst->destReg || !inst->source2Reg) {
            printf("error: invalid register in instruction: '%s'\n", instBuf);
            exit(1);
        }

    } else if (inst->type == FSD) {

        // inst->destReg = stringToArchRegister(strtok(NULL, " \n\t,"));
        inst->source1Reg = stringToArchRegister(strtok(NULL, " \n\t,"));
        inst->imm = atoi(strtok(NULL, " \n\t,()"));
        inst->source2Reg = stringToArchRegister(strtok(NULL, " \n\t,()"));

        // if (!inst->destReg || !inst->source1Reg) {
        if (!inst->source1Reg || !inst->source2Reg) {
            printf("error: invalid register in instruction: '%s'\n", instBuf);
            exit(1);
        }

    // register-immediate instruction
    } else if (inst->type == ADDI) {

        inst->destReg = stringToArchRegister(strtok(NULL, " \n\t,"));
        inst->source1Reg = stringToArchRegister(strtok(NULL, " \n\t,"));
        inst->imm = atoi(strtok(NULL, " \n\t,"));

        if (!inst->destReg || !inst->source1Reg) {
            printf("error: invalid register in instruction: '%s'\n", instBuf);
            exit(1);
        }

    // register-register instructions
    } else if (inst->type == ADD || inst->type == SLT || inst->type == FADD || inst->type == FSUB || inst->type == FMUL || inst->type == FDIV) {
        
        inst->destReg = stringToArchRegister(strtok(NULL, " \n\t,"));
        inst->source1Reg = stringToArchRegister(strtok(NULL, " \n\t,"));
        inst->source2Reg = stringToArchRegister(strtok(NULL, " \n\t,"));

        if (!inst->destReg || !inst->source1Reg || !inst->source2Reg) {
            printf("error: invalid register in instruction: '%s'\n", instBuf);
            exit(1);
        }

    // branch instruction
    } else if (inst->type == BNE) {

        inst->source1Reg = stringToArchRegister(strtok(NULL, " \n\t,"));
        inst->source2Reg = stringToArchRegister(strtok(NULL, " \n\t,"));
        char *targetLabel = strtok(NULL, " \n\t,");
        memcpy(inst->branchTargetLabel, targetLabel, strlen(targetLabel));
        inst->branchTargetLabel[strlen(targetLabel)] = '\0';

        // branch instructions store the pc offset to the target instruction in the imm field (in real implementations this is encoded in instruction already)
        inst->imm = getAddressForLabel(labelTable, inst->branchTargetLabel) - inst->addr;

        #ifdef ENABLE_DEBUG_LOG
        printf("\n\n\ngetAddressForLabel: %i, inst->addr: %i\n\n\n", getAddressForLabel(labelTable, inst->branchTargetLabel), inst->addr);
        #endif

        if (!targetLabel) {
            printf("error: invalid target label in instruction: '%s'\n", instBuf);
            exit(1);
        } else if (!inst->source1Reg || !inst->source2Reg) {
            printf("error: invalid register in instruction: '%s'\n", instBuf);
            exit(1);
        }

    } else {
        printf("error: could not match this point should never be reached...\n");
        exit(1);
    }

    // add the instruction to the decode queue
    decodeUnit->decodeQueue[decodeUnit->numInstsInQueue++] = inst;

    #ifdef ENABLE_DEBUG_LOG
    printf("added instruction: %p to decode queue, numInstsInQueue: %i\n", inst, decodeUnit->numInstsInQueue);
    #endif
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
        #ifdef ENABLE_DEBUG_LOG
        printf("got free register: %s\n", physicalRegisterNameToString(freeRenameReg->reg));
        #endif
        
        // remove the register from the free list
        decodeUnit->freeList = freeRenameReg->next;
        freeRenameReg->next = NULL;
        return freeRenameReg;

    // could not allocated physical register
    } else {
        #ifdef ENABLE_DEBUG_LOG
        printf("error: could not allocate physical register\n");
        #endif

        return NULL;
    }
}

// returns the map table entry associated with a given register
MapTableEntry *mapTableEntryForRegister(DecodeUnit *decodeUnit, ArchRegister *reg) {

    MapTableEntry *curr = decodeUnit->mapTableHead;

    if (!curr || !reg) {
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

    #ifdef ENABLE_DEBUG_LOG
    printf("add physical register: %s to map table for: %s\n", physicalRegisterNameToString(physRegNode->reg), reg->name);
    #endif

    MapTableEntry *mapTableEntry = mapTableEntryForRegister(decodeUnit, reg);

    // the given register currently does not have a map table entry, add it
    if (!mapTableEntry) {
        mapTableEntry = malloc(sizeof(MapTableEntry));
        mapTableEntry->reg = reg;
        mapTableEntry->next = decodeUnit->mapTableHead;
        decodeUnit->mapTableHead = mapTableEntry;
    }

    // add it to the map table
    RegisterMappingNode *regMapHead = mapTableEntry->mapHead;
    physRegNode->next = regMapHead;
    mapTableEntry->mapHead = physRegNode;
}

// gets the rename register currently mapped to a given register
enum PhysicalRegisterName readMapTableForReg(DecodeUnit *decodeUnit, ArchRegister *reg) {

    #ifdef ENABLE_DEBUG_LOG
    printf("read map table for reg: %s\n", reg->name);
    #endif

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
void popOldPhysicalRegisterMappingForReg(DecodeUnit *decodeUnit, ROBStatusTable *robTable, ArchRegister *reg, int addToTail) {

    #ifdef ENABLE_DEBUG_LOG
    printMapTable(decodeUnit);
    #endif

    MapTableEntry *entry = mapTableEntryForRegister(decodeUnit, reg);

    if (!entry) {
        #ifdef ENABLE_DEBUG_LOG
        printf("error: tried to pop oldest physical register mapping for reg: %s which does not have a map table entry\n", reg->name);
        #endif
        
        return;
    }

    // do not pop regiseter if there is only one or zero in the map table
    int numMappings = numPhysicalRegisterMappingsForReg(decodeUnit, reg);
    if (numMappings <= 1) {
        #ifdef ENABLE_DEBUG_LOG
        printf("error: tried to pop a mapping for reg: %s but there are no older mappings\n", reg->name);
        #endif

        return;
    }

    // track the number of "new" writes to the architectural register
    int numNewWritesToReg = 0;

    // get the oldest mapping for the register
    RegisterMappingNode *curr = entry->mapHead;
    RegisterMappingNode *prev = NULL;
    while (curr->next) {
        prev = curr;
        curr = curr->next;

        if (physicalRegWillBeWrittenBySomeROB(robTable, prev->reg)) {
            numNewWritesToReg++;
        }
    }

    // don't pop if there is only one committed mapping
    if (numNewWritesToReg == numMappings - 1) {
        #ifdef ENABLE_DEBUG_LOG
        printf("error: tried to pop a mapping for reg: %s but there is only one committed physical register\n", reg->name);
        #endif

        return;
    }

    // remove the last node from the mapping list
    prev->next = NULL;

    // add physical register to end of the free list
    if (addToTail) {
        RegisterMappingNode *freeListTail = getFreeListTailNode(decodeUnit);

        #ifdef ENABLE_DEBUG_LOG
        printf("adding physical register: %s to the end of the free list\n", physicalRegisterNameToString(curr->reg));
        #endif

        // add the physical register node to the end of the free list
        if (freeListTail) {
            freeListTail->next = curr;
        
        // the free list is currently empty, set the popped node as the head
        } else {
            decodeUnit->freeList = curr;
        }
    
    // add physical register to head of free list
    } else {
        curr->next = decodeUnit->freeList;
        decodeUnit->freeList = curr;
    }
}

// removes the newest physical register in the map table for a given register and adds it back to the free list
void popNewPhysicalRegisterMappingForReg(DecodeUnit *decodeUnit, ArchRegister *reg, int addToTail) {

    MapTableEntry *entry = mapTableEntryForRegister(decodeUnit, reg);

    if (!entry) {
        printf("error: tried to pop newest physical register mapping for reg: %s which does not have a map table entry\n", reg->name);
        return;
    }

    RegisterMappingNode *poppedNode = entry->mapHead;
    
    #ifdef ENABLE_DEBUG_LOG
    if (!poppedNode) {
        printf("error: tried to pop newest node for register: %s but it does not exist\n", reg->name);
    } else {
        printf("popping mapping: %s from map table for: %s\n", physicalRegisterNameToString(poppedNode->reg), reg->name);
    }
    #endif

    entry->mapHead = poppedNode->next;
    poppedNode->next = NULL;

    // add physical register to end of the free list
    if (addToTail) {
        RegisterMappingNode *freeListTail = getFreeListTailNode(decodeUnit);

        #ifdef ENABLE_DEBUG_LOG
        printf("adding physical register: %s to the end of the free list\n", physicalRegisterNameToString(poppedNode->reg));
        #endif

        // add the physical register node to the end of the free list
        if (freeListTail) {
            freeListTail->next = poppedNode;
        
        // the free list is currently empty, set the popped node as the head
        } else {
            decodeUnit->freeList = poppedNode;
        }
    
    // add physical register to head of free list
    } else {
        poppedNode->next = decodeUnit->freeList;
        decodeUnit->freeList = poppedNode;
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
        if (physicalRegisterMappingForReg(decodeUnit, inst->source2Reg) == PHYS_REG_NONE) {
            numNeeded++;
        }

    } else if (inst->type == FSD) {

        // check if dest register doesn't have a mapping
        if (physicalRegisterMappingForReg(decodeUnit, inst->source1Reg) == PHYS_REG_NONE) {
            numNeeded++;
        }

        // check if source register doesn't have a mapping
        if (physicalRegisterMappingForReg(decodeUnit, inst->source2Reg) == PHYS_REG_NONE) {
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
        #ifdef ENABLE_DEBUG_LOG
        printf("error: could not match instruction type during register renaming, this should never happen\n");
        #endif
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
    
    if (inst->type == ADDI) {

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

    } else if (inst->type == FLD) {

        // get register mappings
        enum PhysicalRegisterName source2PhysReg = readMapTableForReg(decodeUnit, inst->source2Reg);

        // check if source register doesn't have a mapping
        if (source2PhysReg == PHYS_REG_NONE) {
            RegisterMappingNode *freeNode = getFreePhysicalRegister(decodeUnit);
            addPhysicalRegisterToMapTable(decodeUnit, freeNode, inst->source2Reg);
            source2PhysReg = freeNode->reg;
        }
        inst->source2PhysReg = source2PhysReg;

        // always get new physical register for this instruction's destinations
        RegisterMappingNode *destFreeNode = getFreePhysicalRegister(decodeUnit);
        addPhysicalRegisterToMapTable(decodeUnit, destFreeNode, inst->destReg);
        inst->destPhysReg = destFreeNode->reg;

    } else if (inst->type == FSD) {

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
        #ifdef ENABLE_DEBUG_LOG
        printf("error: could not match instruction type during register renaming, this should never happen\n");
        #endif
    }

    inst->regsWereRenamed = 1;

    return 1;
}

// execute decode unit's operations during a clock cycle
void cycleDecodeUnit(DecodeUnit *decodeUnit, FetchBufferEntry **fetchBuffer, int *numInstsInBuffer, StatusTables *statusTables, RegisterFile *regFile, StallStats *stallStats, LabelTable *labelTable) {

    #ifdef ENABLE_DEBUG_LOG
    printf("\nperforming decode unit operations...\n");
    #endif

    ROBStatusTable *robTable = statusTables->robTable;
    ResStationStatusTable *resStationTable = statusTables->resStationTable;
    RegisterStatusTable *regTable = statusTables->regTable;

    int numInstsMovedToQueue = 0;

    // loop through all instructions in the input buffer
    for (int i = 0; i < *numInstsInBuffer; i++) {
        FetchBufferEntry *entry = fetchBuffer[i];

        if (!entry) {
            printf("error: was unable to get instruction from decode input buffer when expected\n");
            exit(1);
        }

        // attempt to add instruction to the decode queue
        if (decodeUnit->numInstsInQueue < decodeUnit->NI) {
            addInstructionToDecodeQueue(decodeUnit, labelTable, entry->instStr, entry->instAddr);
            numInstsMovedToQueue++;
        } else {
            // exit loop if queue is full
            break;
        }
    }

    #ifdef ENABLE_DEBUG_LOG
    printf("numInstsInQueue: %i\n", decodeUnit->numInstsInQueue);
    #endif

    // remove instructions from fetch buffer that were moved to the decode queue
    if (numInstsMovedToQueue > 0) {
        for (int i = numInstsMovedToQueue; i < *numInstsInBuffer; i++) {
            free(fetchBuffer[i - numInstsMovedToQueue]); // TODO: keep free here??
            fetchBuffer[i - numInstsMovedToQueue] = fetchBuffer[i];
            fetchBuffer[i] = NULL;
        }
        *numInstsInBuffer -= numInstsMovedToQueue;
    }

    // check if there are any instructions in the decode queue
    if (decodeUnit->numInstsInQueue == 0) {
        #ifdef ENABLE_DEBUG_LOG
        printf("no instructions in decode queue\n");
        #endif

        return;
    }

    /* previous attempt to try to continuously rename registers, it is currently done at instruction issue */
    // printf("attempting to rename instructions...\n");

    // attempt to rename registers for instructions in the decode queue
    // for (int i = 0; i < decodeUnit->numInstsInQueue; i++) {
    //     Instruction *inst = decodeUnit->decodeQueue[i];
    //     enum InstructionType instType = inst->type;

    //     // do not rename instructions that were previously renamed
    //     if (inst->regsWereRenamed) {
    //         continue;
    //     }

    //     // perform the renaming
    //     int renamingWasSuccessful = performRegisterRenamingForInst(decodeUnit, inst);

    //     if (renamingWasSuccessful) {
    //         printf("successfully renamed inst: %p\n", inst);
    //     } else {
    //         printf("could not rename inst: %p, will again next cycle\n", inst);
    //         break;
    //     }
    // }

    #ifdef ENABLE_DEBUG_LOG
    printMapTable(decodeUnit);
    printf("attempting to issue instructions...\n");
    #endif

    int numInstsIssued = 0;

    // attempt to issue up to NW instructions 
    for (int i = 0; i < decodeUnit->NW && numInstsIssued < decodeUnit->numInstsInQueue; i++) {


        Instruction *inst = decodeUnit->decodeQueue[i];
        enum InstructionType instType = inst->type;

        /* leftover from previous attempt to continuously rename registers */
        // stop issuing if the registers were not renamed
        // if (!inst->regsWereRenamed) {
        //     printf("could not issue next instruction: %p as it has not gone through register renaming yet\n", inst);
        //     break;
        // }

        // issue instruction if free slot in ROB and reservation station is available
        if (isFreeEntryInROB(robTable) && isFreeResStationForInstruction(resStationTable, inst)) {
            
            int renamingWasSuccessful = performRegisterRenamingForInst(decodeUnit, inst);
            if (!renamingWasSuccessful) {
                #ifdef ENABLE_DEBUG_LOG
                printf("could not issue next instruction: %p as it has not gone through register renaming yet\n", inst);
                #endif

                break;
            }
            
            numInstsIssued++;

            #ifdef ENABLE_DEBUG_LOG
            printInstruction(*inst);
            printf("renaming was successful\n");
            #endif

            // add entry in the ROB for the given instruction and get the index it is stored at
            int robIndex = addInstToROB(robTable, inst);

            // add the values to the reservation station (just updating the reservation station table)
            addInstToResStation(resStationTable, regTable, regFile, inst, robIndex);

            // store destination register is handled by this function as well
            setRegisterStatusTableEntryROBIndex(regTable, inst->destReg, robIndex);

        } else {

            // check if failure to issue was caused by the ROB being full
            if (!isFreeEntryInROB(robTable)) {
                stallStats->fullROBStalls++;

                #ifdef ENABLE_DEBUG_LOG
                printf("encountered stall in issue unit due to ROB being full, fullROBStalls: %i\n", stallStats->fullROBStalls);
                #endif
            
            // check if the failure to issue was caused by no reservation stations being available
            } else if (!isFreeResStationForInstruction(resStationTable, inst)) {
                stallStats->fullResStationStalls++;

                #ifdef ENABLE_DEBUG_LOG
                printf("encountered stall in issue unit due to reservation stations needed by the instruction being full, fullResStationStalls: %i\n", stallStats->fullResStationStalls);
                #endif
            }

            break;
        }
    }

    // remove issued instructions from the decode queue
    if (numInstsIssued > 0) {
        printf_DEBUG(("removing instructions from decode queue\n"));

        for (int i = numInstsIssued; i < decodeUnit->numInstsInQueue; i++) {
            decodeUnit->decodeQueue[i - numInstsIssued] = decodeUnit->decodeQueue[i];
            decodeUnit->decodeQueue[i] = NULL;
        }

        decodeUnit->numInstsInQueue -= numInstsIssued;
    }
}

// clears all instructions in the fetch buffer and decode queue
void flushDecodeQueue(DecodeUnit *decodeUnit) {

    // clear decode queue
    for (int i = 0; i < decodeUnit->numInstsInQueue; i++) {
        decodeUnit->decodeQueue[i] = NULL;
    }
    decodeUnit->numInstsInQueue = 0;
}