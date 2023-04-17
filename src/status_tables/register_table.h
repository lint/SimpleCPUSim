
// forward declarations
enum PhysicalRegisterName;

// struct representing the register status table
typedef struct RegisterStatusTable {
    int *entries;
    int numEntries;
} RegisterStatusTable;

// register status table methods
void initRegisterStatusTable(RegisterStatusTable *regTable);
void teardownRegisterStatusTable(RegisterStatusTable *regTable);
void printRegisterStatusTable(RegisterStatusTable *regTable);
void setRegisterStatusTableEntryVal(RegisterStatusTable *regTable, enum PhysicalRegisterName reg, int robIndex);
void resetRegisterStatusTableEntryVal(RegisterStatusTable *regTable, enum PhysicalRegisterName reg);
int getRegisterStatusTableEntryVal(RegisterStatusTable *regTable, enum PhysicalRegisterName reg);