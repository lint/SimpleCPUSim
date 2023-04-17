
// forward declarations
typedef struct Params Params; 
typedef struct InstCache InstCache;
typedef struct DataCache DataCache;
typedef struct RegisterFile RegisterFile;
typedef struct FetchUnit FetchUnit;
typedef struct DecodeUnit DecodeUnit;
typedef struct IssueUnit IssueUnit;
typedef struct ROBStatusTable ROBStatusTable;
typedef struct RegisterStatusTable RegisterStatusTable;
typedef struct ResStationStatusTable ResStationStatusTable;

// struct containing information about stall statistics
typedef struct StallStats {

    int fullROBStalls;
    int fullResStationStalls;

} StallStats;

// struct representing the CPU
typedef struct CPU {

    Params *params;
    StallStats *stallStats;

    InstCache *instCache;
    DataCache *dataCache;
    RegisterFile *registerFile;

    ROBStatusTable *robTable;
    RegisterStatusTable *regTable;
    ResStationStatusTable *resStationTable;

    FetchUnit *fetchUnit;
    DecodeUnit *decodeUnit;
    IssueUnit *issueUnit;

} CPU;

void initCPU(CPU *cpu, Params *params);
void teardownCPU(CPU *cpu);
void executeCPU(CPU *cpu);