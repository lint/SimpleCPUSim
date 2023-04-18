
// forward declarations
typedef struct IntFUResult IntFUResult;
typedef struct StatusTables StatusTables;

typedef struct IntFunctionalUnit {

    IntFUResult **stages;
    int lastSelectedResStation;
    int latency;
    int fuType; // FunctionalUnitType enum
    int isStalled; // operations get stalled because the writeback unit could not write the result on the CDB

} IntFunctionalUnit;

// int functional unit methods
void initIntFunctionalUnit(IntFunctionalUnit *intFU);
void teardownIntFunctionalUnit(IntFunctionalUnit *intFU);
IntFUResult *getCurrentIntFunctionalUnitResult(IntFunctionalUnit *intFU);
void printIntFunctionalUnit(IntFunctionalUnit *intFU);
void cycleIntFunctionalUnit(IntFunctionalUnit *intFU, StatusTables *statusTables);