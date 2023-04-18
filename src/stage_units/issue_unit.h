
// forward declarations
typedef struct Instruction Instruction;
typedef struct StatusTables StatusTables;
typedef struct RegisterFile RegisterFile;
typedef struct StallStats StallStats;

// struct representing a decode unit
typedef struct IssueUnit {
    Instruction **instDecodeQueue;
    int *numInstsInQueue;
    int numInstsIssued;
    int NW;
    int NI;
} IssueUnit;

// issue unit methods
void initIssueUnit(IssueUnit *issueUnit, int NW, int NI, Instruction **instDecodeQueue, int *numInstsInQueue);
void teardownIssueUnit(IssueUnit *issueUnit);
void cycleIssueUnit(IssueUnit *issueUnit, StatusTables *statusTables, RegisterFile *regFile, StallStats *stallStats);