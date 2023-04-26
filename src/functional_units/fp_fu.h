
// forward declarations
typedef struct FloatFUResult FloatFUResult;
typedef struct StatusTables StatusTables;

typedef struct FPFunctionalUnit {
    FloatFUResult **stages;
    int lastSelectedResStation;
    int latency;
    int fuType; // FunctionalUnitType enum
    int isStalled; // operations get stalled because the writeback unit could not write the result on the CDB
} FPFunctionalUnit;

// int functional unit methods
void initFPFunctionalUnit(FPFunctionalUnit *fpFU, int fuType, int latency);
void teardownFPFunctionalUnit(FPFunctionalUnit *fpFU);
FloatFUResult *getCurrentFPFunctionalUnitResult(FPFunctionalUnit *fpFU);
void printFPFunctionalUnit(FPFunctionalUnit *fpFU);
void flushFPFunctionalUnit(FPFunctionalUnit *fpFU);
void cycleFPFunctionalUnit(FPFunctionalUnit *fpFU, StatusTables *statusTables);