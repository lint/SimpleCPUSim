
// forward declarations
typedef struct BUFUResult BUFUResult;
typedef struct StatusTables StatusTables;

typedef struct BUFunctionalUnit {
    BUFUResult **stages;
    int lastSelectedResStation;
    int latency;
    int fuType; // FunctionalUnitType enum
    int isStalled; // operations get stalled because the writeback unit could not write the result on the CDB
} BUFunctionalUnit;

// int functional unit methods
void initBUFunctionalUnit(BUFunctionalUnit *buFU, int latency);
void teardownBUFunctionalUnit(BUFunctionalUnit *buFU);
BUFUResult *getCurrentBUFunctionalUnitResult(BUFunctionalUnit *buFU);
void printBUFunctionalUnit(BUFunctionalUnit *buFU);
void flushBUFunctionalUnit(BUFunctionalUnit *buFU);
void cycleBUFunctionalUnit(BUFunctionalUnit *buFU, StatusTables *statusTables);