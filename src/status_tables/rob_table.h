
// forward declarations
typedef struct Instruction Instruction;
enum InstructionState;
enum RegisterName;

// struct representing an entry in the ROB status table
typedef struct ROBStatusTableEntry {

    int index; // the index of the entry in the table (also implicitly defined by entry ordering)
    int busy;
    Instruction *inst;
    int state; // InstructionState enum
    int dest; // PhysicalRegisterName enum for load and ALU operations OR WAIT address for stores as well?????
    int intValue;
    float floatValue;
    int instResultValueType; // InstructionResultValueType enum

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