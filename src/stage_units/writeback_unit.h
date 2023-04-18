
// forward declarations
typedef struct ResStationStatusTableEntry ResStationStatusTableEntry;
typedef struct StatusTables StatusTables;
typedef struct FunctionalUnits FunctionalUnits;

// struct representing a common data bus
typedef struct CDB {
    int intVal;
    float floatVal;
    int robIndex;
    int containsData;
} CDB;

// struct containing information about each ROB entry to make decisions about what values to place on the CDB
typedef struct ROBWBInfo {

    int intVal;
    float floatVal;
    int robIndex;
    int numOperandsWaiting;
    int producedBy; // enum ValProducedBy
    int producingFUType; // enum FunctionalUnitType
    int valueType; // enum InstructionResultValueType

} ROBWBInfo;

// struct representing the writeback unit
typedef struct WritebackUnit {

    CDB **cdbs;
    ROBWBInfo **robWBInfoArr;
    int cdbsUsed;
    int NB;
    int NR;

} WritebackUnit;

// writeback unit methods
void initWritebackUnit(WritebackUnit *writebackUnit, int NB, int NR);
void teardownWritebackUnit(WritebackUnit *writebackUnit);
void resetWritebackUnitROBWBInfo(WritebackUnit *writebackUnit);
void updateWritebackUnitWaitingForROB(WritebackUnit *writebackUnit, ResStationStatusTableEntry **resStationEntries, int numEntries);
void cycleWritebackUnit(WritebackUnit *writebackUnit, StatusTables *statusTables, FunctionalUnits *functionalUnits);
void printWritebackUnitROBInfo(WritebackUnit *writebackUnit);
void printWritebackUnitCDBS(WritebackUnit *writebackUnit);
