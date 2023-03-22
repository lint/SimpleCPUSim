
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
    NONEI = -1,
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

// enum representing the name of RISC-V registers
enum RegisterName {
    NONER = -1, 
    R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, R14, R15, R16, R17, R18, R19, R20, R21, R22, R23, R24, R25, R26, R27, R28, R29, R30, R31,
    F0, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24, F25, F26, F27, F28, F29, F30, F31,
    PC,
    ZERO,
    RN_SIZE
};

// enum representing the name of physical registers
enum PhysicalRegisterName {
    p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31,
    PHYS_RN_SIZE
};

// struct representing a node storing register information for the mapping table and free list
typedef struct RegisterMappingNode {
    enum PhysicalRegisterName reg;
    struct RegisterMappingNode *next;
} RegisterMappingNode;

// struct representing an instruction
typedef struct Instruction {
    enum InstructionType type;
    enum RegisterName source1Reg;
    enum RegisterName source2Reg;
    enum RegisterName destReg;
    enum PhysicalRegisterName source1PhysReg;
    enum PhysicalRegisterName source2PhysReg;
    enum PhysicalRegisterName destPhysReg;
    int imm; // either the offset used in load/store instructions or an immediate for addi instructions
    int branchTargetAddr;
    char branchTargetLabel[256];
    char label[256];
} Instruction;

// helper methods to convert strings to enums
enum InstructionType stringToInstructionType(char *s);
enum RegisterName stringToRegisterName(char *s);
