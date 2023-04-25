

// struct containing parameters read from a config file
typedef struct Params {
    int NF; // instructions fetched per cycle
    int NI; // decoded instruction queue length
    int NW; // instructions issued to reservation stations per cycle
    int NR; // circular ROB entries
    int NB; // common data busses 
} Params;

// struct representing an architectural register
typedef struct ArchRegister {
    char name[8];
    int num;
    int regType; // enum ArchRegisterType
} ArchRegister;

// struct representing an instruction
typedef struct Instruction {
    int type; // enum InstructionType
    ArchRegister *source1Reg;
    ArchRegister *source2Reg;
    ArchRegister *destReg;
    int source1PhysReg; // enum PhysicalRegisterName
    int source2PhysReg; // enum PhysicalRegisterName
    int destPhysReg; // enum PhysicalRegisterName
    int regsWereRenamed;
    int imm; // either the offset used in load/store instructions or an immediate for addi instructions
    int branchTargetAddr; // TODO: remove this?
    char branchTargetLabel[256];
    char label[256];
    int addr; // address in the instruction cache
    char fullStr[256];
} Instruction;

// helper methods
int stringToInstructionType(char *s);
char *physicalRegisterNameToString(int reg); // reg = enum PhysicalRegisterName
void printInstruction(Instruction inst);
char *instStateToString(int state); // state = enum InstructionState
char *valueTypeToString(int type); // type = enum ValueType
char *fuOpToString(int op); // op = enum FunctionalUnitOperations
char *fuTypeToString(int fuType); // fuType = enum FunctionalUnitType 
char *valueProducedByToString(int producedBy); // producedBy = enum ValueProducedBy
char *branchPredictionStateToString(int state); // state = enum BranchPredictionState
ArchRegister *stringToArchRegister(char *s);
int archRegistersAreEqual(ArchRegister *reg1, ArchRegister *reg2);