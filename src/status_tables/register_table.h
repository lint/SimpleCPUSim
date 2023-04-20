
// forward declarations
typedef struct ArchRegister ArchRegister;

// struct representing an entry in the register status table
typedef struct RegisterStatusTableEntry {
    int robIndex;
    ArchRegister *reg;
    struct RegisterStatusTableEntry *next;
} RegisterStatusTableEntry;

// struct representing the register status table
typedef struct RegisterStatusTable {
    RegisterStatusTableEntry *headEntry;
} RegisterStatusTable;

// register status table methods
void initRegisterStatusTable(RegisterStatusTable *regTable);
void teardownRegisterStatusTable(RegisterStatusTable *regTable);
void printRegisterStatusTable(RegisterStatusTable *regTable);
void setRegisterStatusTableEntryROBIndex(RegisterStatusTable *regTable, ArchRegister *reg, int robIndex);
int getRegisterStatusTableEntryROBIndex(RegisterStatusTable *regTable, ArchRegister *reg);
RegisterStatusTableEntry *registerStatusTableEntryForReg(RegisterStatusTable *regTable, ArchRegister *reg);