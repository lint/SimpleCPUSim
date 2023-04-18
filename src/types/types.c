
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "types.h"

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
        return NONEI;
    }
}

// converts a string to the register name enum
enum RegisterName stringToRegisterName(char *s) {

    // printf("getting register enum for: %s\n", s);

    int regNum = -1;
    int offset = s[0] == 'R' ? 0 : 32; // R registers are enum values 0 - 31, F registers are enum values 32 - 63
    
    if (!strcmp(s, "PC")) {
        return PC;
    } else if (!strcmp(s, "$0")) {
        return ZERO;
    }

    if (sscanf(s + 1, "%d", &regNum) <= 0) {
        return NONER;
    }

    return (enum RegisterName)(regNum + offset);
}

// converts the register name enum to string
char *registerEnumToString(enum RegisterName reg) {

    if (reg == PC) {
        return "PC";
    } else if (reg == ZERO) {
        return "$0";
    }

    char *str = malloc(4 * sizeof(char)); // potential memory leak if not freed
    char regType = reg < 32 ? 'R' : 'F';
    int regNum = reg < 32 ? reg : reg - 32;

    sprintf(str, "%c%d", regType, regNum);

    return str;
}

// converts the physical register name enum to string
char *physicalRegisterEnumToString(enum PhysicalRegisterName reg) {

    if (reg == PHYS_PC) {
        return "PHYS_PC";
    } else if (reg == PHYS_ZERO) {
        return "p_$0";
    }

    char *str = malloc(4 * sizeof(char));
    sprintf(str, "p%d", reg);

    return str;
}

// converts the instruction state enum to a string
char *instStateToString(enum InstructionState state) {
    if (state == STATE_ISSUED) {
        return "ISSUED";
    } else if (state == STATE_EXECUTING) {
        return "EXECUTING";
    } else if (state == STATE_WROTE_RESULT) {
        return "WROTE_RESULT";
    } else if (state == STATE_COMMIT) {
        return "COMMIT";
    } else {
        return "NONE";
    }
}

// converts the instruction result value type to string
char *instResultTypeToString(enum InstructionResultValueType type) {
    if (type == INST_VAL_FLOAT) {
        return "float";
    } else if (type == INST_VAL_INT) {
        return "int";
    } else {
        return "NONE";
    }
}

// converts the functional unit operations enum to string
char *fuOpToString(enum FunctionalUnitOperations op) {
    if (op == FUOp_ADD) {
        return "add";
    } else if (op == FUOp_SUB) {
        return "sub";
    } else if (op == FUOp_SLT) {
        return "slt";
    } else {
        return "NONE";
    }
}

// converts the functional unit type to string
char *fuTypeToString(enum FunctionalUnitType fuType) {
    if (fuType == FUType_LOAD) {
        return "FU_FSD";
    } else if (fuType == FUType_STORE) {
        return "FU_STORE";
    } else if (fuType == FUType_INT) {
        return "FU_INT";
    } else if (fuType == FUType_FPAdd) {
        return "FU_FPADD";
    } else if (fuType == FUType_FPMult) {
        return "FU_FPMULT";
    } else if (fuType == FUType_FPDiv) {
        return "FU_FPDIV";
    } else if (fuType == FUType_BU) {
        return "FU_BU";
    } else {
        return "NONE";
    }
}

// converts the val produced by enum to string
char *valProducedByToString(enum ValProducedBy producedBy) {
    if (producedBy == VAL_FROM_FU) {
        return "VAL_FROM_FU";
    } else if (producedBy == VAL_FROM_ROB) {
        return "VAL_FROM_ROB";
    } else {
        return "NONE";
    }
}

// prints the contents of an instruction
void printInstruction(Instruction inst) {
    printf("instruction: label: %s, type: %i, imm: %i, destReg: %i, sourceReg1: %i, sourceReg2: %i, destPhysReg: %i, source1PhysReg: %i, source2PhysReg: %i, branchTargetAddr: %i, branchTargetLabel: %s\n", 
                inst.label, inst.type, inst.imm, inst.destReg, inst.source1Reg, inst.source2Reg, inst.destPhysReg, inst.source1PhysReg, inst.source2PhysReg, inst.branchTargetAddr, inst.branchTargetLabel);
}