
// forward declarations
typedef struct Instruction Instruction;
typedef struct ROBStatusTable ROBStatusTable;
typedef struct ResStationStatusTable ResStationStatusTable;
typedef struct RegisterStatusTable RegisterStatusTable;
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

void initIssueUnit(IssueUnit *issueUnit, int NW, int NI, Instruction **instDecodeQueue, int *numInstsInQueue);
void teardownIssueUnit(IssueUnit *issueUnit);
void cycleIssueUnit(IssueUnit *issueUnit, RegisterFile *regFile, ROBStatusTable *robTable, ResStationStatusTable *resStationTable, RegisterStatusTable *regTable, StallStats *stallStats);