
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "types.h"
#include "enums.h"

// converts a string to the instruction type enum
enum InstructionType stringToInstructionType(char *s) {
    // printf("getting type for: %s\n", s);

    if (!strcmp(s, "fld")) {
        return FLD;
    } else if (!strcmp(s, "fsd")) {
        return FSD;
    } else if (!strcmp(s, "add")) {
        return ADD;
    } else if (!strcmp(s, "addi")) {
        return ADDI;
    } else if (!strcmp(s, "slt")) {
        return SLT;
    } else if (!strcmp(s, "fadd")) {
        return FADD;
    } else if (!strcmp(s, "fsub")) {
        return FSUB;
    } else if (!strcmp(s, "fmul")) {
        return FMUL;
    } else if (!strcmp(s, "fdiv")) {
        return FDIV;
    } else if (!strcmp(s, "bne")) {
        return BNE;
    } else {
        return INST_TYPE_NONE;
    }
}

// converts an instruction type enum to string
char *InstructionTypeToString(enum InstructionType instType) {

    if (instType == FLD) {
        return "fld";
    } else if (instType == FSD) {
        return "fsd";
    } else if (instType == ADD) {
        return "add";
    } else if (instType == ADDI) {
        return "addi";
    } else if (instType == SLT) {
        return "slt";
    } else if (instType == FADD) {
        return "fadd";
    } else if (instType == FSUB) {
        return "fsub";
    } else if (instType == FMUL) {
        return "fmul";
    } else if (instType == FDIV) {
        return "fdiv";
    } else if (instType == BNE) {
        return "bne";
    } else {
        return "NONE";
    }
}

// converts the rename register name enum to string
char *physicalRegisterNameToString(enum PhysicalRegisterName reg) {

    if (reg < 0) {
        return "NONE";
    } else if (reg == PHYS_REG_PC) {
        return "PHYS_PC";
    } else if (reg == PHYS_REG_ZERO) {
        return "PHYS_ZERO";
    }

    char *str = malloc(4 * sizeof(char));
    sprintf(str, "p%d", reg);

    return str;
}

// converts the instruction state enum to a string
char *instStateToString(enum InstructionState state) {
    if (state == INST_STATE_ISSUED) {
        return "ISSUED";
    } else if (state == INST_STATE_EXECUTING) {
        return "EXECUTING";
    } else if (state == INST_STATE_WROTE_RESULT) {
        return "WROTE_RESULT";
    } else if (state == INST_STATE_COMMIT) {
        return "COMMIT";
    } else {
        return "NONE";
    }
}

// converts the value type to string
char *valueTypeToString(enum ValueType type) {
    if (type == VALUE_TYPE_FLOAT) {
        return "float";
    } else if (type == VALUE_TYPE_INT) {
        return "int";
    } else {
        return "NONE";
    }
}

// converts the functional unit operations enum to string
char *fuOpToString(enum FunctionalUnitOperations op) {
    if (op == FU_OP_ADD) {
        return "add";
    } else if (op == FU_OP_SUB) {
        return "sub";
    } else if (op == FU_OP_SLT) {
        return "slt";
    } else {
        return "NONE";
    }
}

// converts the functional unit type to string
char *fuTypeToString(enum FunctionalUnitType fuType) {
    if (fuType == FU_TYPE_LOAD) {
        return "FU_LOAD";
    } else if (fuType == FU_TYPE_STORE) {
        return "FU_STORE";
    } else if (fuType == FU_TYPE_INT) {
        return "FU_INT";
    } else if (fuType == FU_TYPE_FPADD) {
        return "FU_FPADD";
    } else if (fuType == FU_TYPE_FPMUL) {
        return "FU_FPMUL";
    } else if (fuType == FU_TYPE_FPDIV) {
        return "FU_FPDIV";
    } else if (fuType == FU_TYPE_BU) {
        return "FU_BU";
    } else {
        return "NONE";
    }
}

// converts the val produced by enum to string
char *valueProducedByToString(enum ValueProducedBy producedBy) {
    if (producedBy == VALUE_FROM_FU) {
        return "VALUE_FROM_FU";
    } else if (producedBy == VALUE_FROM_ROB) {
        return "VALUE_FROM_ROB";
    } else {
        return "NONE";
    }
}

// converts the branch predictor state to string
char *branchPredictionStateToString(enum BranchPredictionState state) {
    if (state == BRANCH_STATE_STRONGLY_NOT_TAKEN) {
        return "STRONGLY_NOT_TAKEN";
    } else if (state == BRANCH_STATE_STRONGLY_TAKEN) {
        return "STRONGLY_TAKEN";
    } else if (state == BRANCH_STATE_WEAKLY_NOT_TAKEN) {
        return "WEAKLY_NOT_TAKEN";
    } else if (state == BRANCH_STATE_WEAKLY_TAKEN) {
        return "WEAKLY_TAKEN";
    } else {
        return "NONE";
    }
}

// prints the contents of an instruction
void printInstruction(Instruction inst) {
    printf("instruction: addr: %i, label: %s, type: %i, imm: %i, destReg: %s, sourceReg1: %s, sourceReg2: %s, ", 
        inst.addr, inst.label, inst.type, inst.imm, inst.destReg->name, inst.source1Reg->name, inst.source2Reg->name);
    printf("renamedDest: %s, renamedSource1: %s, renamedSource2: %s, branchTargetLabel: %s\n", physicalRegisterNameToString(inst.destPhysReg), 
        physicalRegisterNameToString(inst.source1PhysReg), physicalRegisterNameToString(inst.source2PhysReg), inst.branchTargetLabel);
}

// converts a given string into an arch register instance which is more easily compared
ArchRegister *stringToArchRegister(char *s) {

    ArchRegister *reg = malloc(sizeof(ArchRegister));
    strcpy(reg->name, s);

    if (!strcmp(s, "PC")) {
        reg->regType = ARCH_REG_PC;
        reg->num = -1;
    } else if (!strcmp(s, "$0")) {
        reg->regType = ARCH_REG_ZERO;
        reg->num = -1;
    } else {
        if (s[0] == 'R') {
            reg->regType = VALUE_TYPE_INT;
        } else if (s[0] == 'F') {
            reg->regType = VALUE_TYPE_FLOAT;
        } else {
            #ifdef ENABLE_DEBUG_LOG
            printf("error: could not create ArchRegister struct for the given input register: %s\n", s);
            #endif

            free(reg);
            return NULL;
        }

        if (sscanf(s + 1, "%d", &reg->num) <= 0) {
            #ifdef ENABLE_DEBUG_LOG
            printf("error: could not create ArchRegister struct for the given input register: '%s' due to an invalid\n", s);
            #endif

            return NULL;
        }
    }
    
    return reg;
} 

// helper method which determines if two ArchRegister structs are equal
int archRegistersAreEqual(ArchRegister *reg1, ArchRegister *reg2) {
    return reg1->regType == reg2->regType && reg1->num == reg2->num;
}