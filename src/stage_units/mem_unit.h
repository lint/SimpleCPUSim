
// forward declarations
typedef struct DataCache DataCache;
typedef struct StatusTables StatusTables;
typedef struct LSFUResult LSFUResult;
typedef struct LSFunctionalUnit LSFunctionalUnit;

// struct representing the memory unit
typedef struct MemoryUnit {

    LSFUResult *currResult;
    int isStalledFromWB;
    int isStalledFromStore;
    float forwardedData;
    int forwardedAddr;

} MemoryUnit;

// memory unit methods
void initMemoryUnit(MemoryUnit *memUnit);
void teardownMemoryUnit(MemoryUnit *memUnit);
void cycleMemoryUnit(MemoryUnit *memUnit, DataCache *dataCache, LSFunctionalUnit *lsFU, StatusTables *statusTables);
void flushMemUnit(MemoryUnit *memUnit);
void printMemoryUnit(MemoryUnit *memUnit);
LSFUResult *getMemoryUnitCurrentResult(MemoryUnit *memUnit);
void forwardDataToMemoryUnit(MemoryUnit *memUnit, float value, int destROB);
void clearMemoryUnitForwardedData(MemoryUnit *memUnit);