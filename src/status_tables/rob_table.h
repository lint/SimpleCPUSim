
// forward declarations
typedef struct Instruction Instruction;
typedef struct ArchRegister ArchRegister;

// struct representing an entry in the ROB status table
typedef struct ROBStatusTableEntry {

    int index; // the index of the entry in the table
    int busy;
    Instruction *inst;
    int state; // enum InstructionState
    ArchRegister *destReg; // destination architectural register for load and ALU operations
    int renamedDestReg;
    int intValue;
    float floatValue;
    int instResultValueType; // enum InstructionResultValueType
    int fuType; // the type of the functional unit associated with this result
    int addr; // contains the address used in stores / branches
    int flushed; // indicates whether this entry in the ROB was flushed (just used for printing)

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
void printROBStatusTable(ROBStatusTable *robTable);
int isFreeEntryInROB(ROBStatusTable *robTable);
int nextFreeROBEntryIndex(ROBStatusTable *robTable);
int addInstToROB(ROBStatusTable *robTable, Instruction *inst);
ROBStatusTableEntry *getHeadROBEntry(ROBStatusTable *robTable);
int isROBEmpty(ROBStatusTable *robTable);
void flushROB(ROBStatusTable *robTable);
int indexDistanceToROBHead(ROBStatusTable *robTable, int index);
int physicalRegWillBeWrittenBySomeROB(ROBStatusTable *robTable, int reg); // reg = enum PhysicalRegisterName