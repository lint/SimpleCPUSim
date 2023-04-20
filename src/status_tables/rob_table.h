
// forward declarations
typedef struct Instruction Instruction;
typedef struct ArchRegister ArchRegister;

// struct representing an entry in the ROB status table
typedef struct ROBStatusTableEntry {

    int index; // the index of the entry in the table (also implicitly defined by entry ordering)
    int busy;
    Instruction *inst;
    int state; // InstructionState enum
    ArchRegister *destReg; // destination register for load and ALU operations
    int renamedDestReg;
    int intValue;
    float floatValue;
    int instResultValueType; // InstructionResultValueType enum

    // TODO: need something to store the address for stores

} ROBStatusTableEntry;

// struct representing the ROB status table
typedef struct ROBStatusTable {
    ROBStatusTableEntry **entries;
    int NR;
    int headEntryIndex;
} ROBStatusTable;

// ROB status table methods
void initROBStatusTable(ROBStatusTable *robTable, int NR);
void teardownROBStatusTable(ROBStatusTable *robTable);
int isFreeEntryInROB(ROBStatusTable *robTable);
int nextFreeROBEntryIndex(ROBStatusTable *robTable);
int addInstToROB(ROBStatusTable *robTable, Instruction *inst);
void printROBStatusTable(ROBStatusTable *robTable);
ROBStatusTableEntry *getHeadROBEntry(ROBStatusTable *robTable);
int isROBEmpty(ROBStatusTable *robTable);