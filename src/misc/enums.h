
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

// enum representing what source produced a certain value
enum ValueProducedBy {
    VALUE_FROM_NONE = -1,
    VALUE_FROM_ROB,
    VALUE_FROM_FU,
    VALUE_FROM_MEM_UNIT
};

// enum representing the different types of possible architectural registers
enum ArchRegisterType {
    ARCH_REG_NONE = -1,
    ARCH_REG_INT,
    ARCH_REG_FLOAT,
    ARCH_REG_PC,
    ARCH_REG_ZERO
};

// enum representing the name of a physical register
enum PhysicalRegisterName {
    PHYS_REG_NONE = -1,
    p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31,
    PHYS_REG_SIZE,
    PHYS_REG_PC,
    PHYS_REG_ZERO
};

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
    INST_STATE_NONE = -1,
    INST_STATE_ISSUED,
    INST_STATE_EXECUTING,
    INST_STATE_WROTE_RESULT,
    INST_STATE_COMMIT
};

// enum representing the different states that a branch predictor can be in
enum BranchPredictionState {
    BRANCH_STATE_NONE = -1,
    BRANCH_STATE_STRONGLY_TAKEN,
    BRANCH_STATE_WEAKLY_TAKEN,
    BRANCH_STATE_WEAKLY_NOT_TAKEN,
    BRANCH_STATE_STRONGLY_NOT_TAKEN,
};