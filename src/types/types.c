
#include <stdio.h>
#include <string.h>
#include "types.h"

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

void printInstruction(Instruction inst) {
    printf("instruction: label: %s, type: %i, imm: %i, destReg: %i, sourceReg1: %i, sourceReg2: %i, destPhysReg: %i, source1PhysReg: %i, source2PhysReg: %i, branchTargetAddr: %i, branchTargetLabel: %s\n", 
                inst.label, inst.type, inst.imm, inst.destReg, inst.source1Reg, inst.source2Reg, inst.destPhysReg, inst.source1PhysReg, inst.source2PhysReg, inst.branchTargetAddr, inst.branchTargetLabel);
}