
// forward declarations
typedef struct IntFUResult IntFUResult;

typedef struct IntFunctionalUnit {

    IntFUResult **stages;
    int lastSelectedResStation;
    int latency;
    int fuType; // FunctionalUnitType enum

} IntFunctionalUnit;

// int functional unit methods
void initIntFunctionalUnit(IntFunctionalUnit *intFU);
void teardownIntFunctionalUnit(IntFunctionalUnit *intFU);
IntFUResult *getCurrentIntFunctionalUnitResult(IntFunctionalUnit *intFU);
void printIntFunctionalUnit(IntFunctionalUnit *intFU);
void cycleIntFunctionalUnit(IntFunctionalUnit *intFU, ResStationStatusTable *resStationTable, ROBStatusTable *robTable);