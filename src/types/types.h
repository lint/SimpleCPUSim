
#include "functional_unit_types.h"
#include "status_table_types.h"

// struct containing parameters read from a config file
typedef struct Params {
    int NF; // instructions fetched per cycle
    int NI; // decoded instruction queue length
    int NW; // instructions issued to reservation stations per cycle
    int NR; // circular ROB entries
    int NB; // common data busses 
} Params;

// enum representing the type of each supported instruction
enum InstructionType {
    INST_TYPE_NONE = -1,
    FLD, 
    FSD, 
    ADD, 
    ADDI, 
    SLT, 
    FADD, 
    FSUB, 
    FMUL, 
    FDIV, 
    BNE
};

// enum representing the different types of values that an instruction can result in
enum ValueType {
    VALUE_TYPE_NONE = -1,
    VALUE_TYPE_INT,
    VALUE_TYPE_FLOAT
};

enum ValueProducedBy {
    VALUE_FROM_NONE = -1,
    VALUE_FROM_ROB,
    VALUE_FROM_FU
};

enum ArchRegisterType {
    ARCH_REG_NONE = -1,
    ARCH_REG_INT,
    ARCH_REG_FLOAT,
    ARCH_REG_PC,
    ARCH_REG_ZERO
};

typedef struct ArchRegister {
    char name[8];
    int num;
    enum ArchRegisterType regType;
} ArchRegister;

enum PhysicalRegisterName {
    PHYS_REG_NONE = -1,
    p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31,
    PHYS_REG_SIZE,
    PHYS_REG_PC,
    PHYS_REG_ZERO
};

// struct representing a node storing register information for the mapping table and free list
typedef struct RegisterMappingNode {
    // enum PhysicalRegisterName reg;
    enum PhysicalRegisterName reg;
    struct RegisterMappingNode *next;
} RegisterMappingNode;

// struct representing an instruction
typedef struct Instruction {
    enum InstructionType type;
    ArchRegister *source1Reg;
    ArchRegister *source2Reg;
    ArchRegister *destReg;
    enum PhysicalRegisterName source1PhysReg;
    enum PhysicalRegisterName source2PhysReg;
    enum PhysicalRegisterName destPhysReg;
    int regsWereRenamed;
    int imm; // either the offset used in load/store instructions or an immediate for addi instructions
    int branchTargetAddr;
    char branchTargetLabel[256];
    char label[256];
    int addr; // address in the instruction cache
} Instruction;

// enum representing the different functional unit types
enum FunctionalUnitType {
    FU_TYPE_NONE = -1,
    FU_TYPE_INT,
    FU_TYPE_LOAD,
    FU_TYPE_STORE,
    FU_TYPE_FPADD,
    FU_TYPE_FPMUL,
    FU_TYPE_FPDIV,
    FU_TYPE_BU
};

// enum representing operations that different functional units can perform
enum FunctionalUnitOperations {
    FU_OP_NONE = -1,
    FU_OP_ADD,
    FU_OP_SLT,
    FU_OP_SUB
};

// enum represneting the different states that an instruction can be in
enum InstructionState {
    STATE_NONE = -1,
    STATE_ISSUED,
    STATE_EXECUTING,
    STATE_WROTE_RESULT,
    STATE_COMMIT
};



// helper methods
enum InstructionType stringToInstructionType(char *s);
char *physicalRegisterNameToString(enum PhysicalRegisterName reg);
void printInstruction(Instruction inst);
char *instStateToString(enum InstructionState state);
char *valueTypeToString(enum ValueType type);
char *fuOpToString(enum FunctionalUnitOperations op);
char *fuTypeToString(enum FunctionalUnitType fuType);
char *valueProducedByToString(enum ValueProducedBy producedBy);
ArchRegister *stringToArchRegister(char *s);
int archRegistersAreEqual(ArchRegister *reg1, ArchRegister *reg2);