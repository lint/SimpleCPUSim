
// forward declarations
typedef struct Params Params; 
typedef struct InstCache InstCache;
typedef struct DataCache DataCache;
typedef struct RegisterFile RegisterFile;
typedef struct FetchUnit FetchUnit;
typedef struct DecodeUnit DecodeUnit;
typedef struct IssueUnit IssueUnit;
typedef struct WritebackUnit WritebackUnit;
typedef struct StatusTables StatusTables;
typedef struct ROBStatusTable ROBStatusTable;
typedef struct RegisterStatusTable RegisterStatusTable;
typedef struct ResStationStatusTable ResStationStatusTable;
typedef struct FunctionalUnits FunctionalUnits;
typedef struct IntFunctionalUnit IntFunctionalUnit;

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

    FetchUnit *fetchUnit;
    DecodeUnit *decodeUnit;
    IssueUnit *issueUnit;
    WritebackUnit *writebackUnit;

    FunctionalUnits *functionalUnits;
    StatusTables *statusTables;

} CPU;

void initCPU(CPU *cpu, Params *params);
void teardownCPU(CPU *cpu);
void executeCPU(CPU *cpu);