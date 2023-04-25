
// forward declarations
typedef struct StatusTables StatusTables;
typedef struct LSFUResult LSFUResult;

typedef struct LSFunctionalUnit {

    LSFUResult **stages;
    int latency;
    int fuType; // FunctionalUnitType enum
    int isStalled; // operations get stalled because the writeback unit could not write the result on the CDB

} LSFunctionalUnit;

// load/store functional unit methods
void initLSFunctionalUnit(LSFunctionalUnit *lsFU, int latency);
void teardownLSFunctionalUnit(LSFunctionalUnit *lsFU);
LSFUResult *getCurrentLSFunctionalUnitResult(LSFunctionalUnit *lsFU);
void printLSFunctionalUnit(LSFunctionalUnit *lsFU);
void flushLSFunctionalUnit(LSFunctionalUnit *lsFU);
void cycleLSFunctionalUnit(LSFunctionalUnit *lsFU, StatusTables *statusTables);