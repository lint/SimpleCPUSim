
// forward declarations
typedef struct RegisterFile RegisterFile;
typedef struct RegisterStatusTable RegisterStatusTable;
typedef struct Instruction Instruction;
typedef struct FloatFUResult FloatFUResult;
typedef struct IntFUResult IntFUResult;

// struct representing the reservation status table's entries
typedef struct ResStationStatusTableEntry {
    int fuType; // enum FunctionalUnitType
    int resStationIndex;
    int busy;
    int op; // enum FunctionalUnitOperation
    int vjInt;
    int vkInt;
    float vjFloat;
    float vkFloat;
    int vjIsAvailable;
    int vkIsAvailable;
    int qj; // ROB index containing source operand 1
    int qk; // ROB index containing source operand 2
    int dest; // the ROB index that will hold the result
    int addr; // the stored effective address (used with loads/stores)

} ResStationStatusTableEntry;

// struct representing the reservation status table
typedef struct ResStationStatusTable {

    ResStationStatusTableEntry **intEntries;
    ResStationStatusTableEntry **loadEntries;
    ResStationStatusTableEntry **storeEntries;
    ResStationStatusTableEntry **fpAddEntries;
    ResStationStatusTableEntry **fpMultEntries;
    ResStationStatusTableEntry **fpDivEntries;
    ResStationStatusTableEntry **buEntries;
    
    int numIntStations;
    int numLoadStations;
    int numStoreStations;
    int numFPAddStations;
    int numFPMultStations;
    int numFPDivStations;
    int numBUStations;

} ResStationStatusTable;

// reservation status table methods
ResStationStatusTableEntry *newResStationStatusTableEntry();
void initResStationStatusTable(ResStationStatusTable *resStationTable);
void teardownResStationStatusTable(ResStationStatusTable *resStationTable);
int numResStationsForFunctionalUnit(ResStationStatusTable *resStationTable, int fuType);
int numBusyResStationsForFunctionalUnit(ResStationStatusTable *resStationTable, int fuType);
ResStationStatusTableEntry **resStationEntriesForFunctionalUnit(ResStationStatusTable *resStationTable, int fuType);
ResStationStatusTableEntry *resStationEntryForFunctionalUnitWithDestROB(ResStationStatusTable *resStationTable, int fuType, int destROB);
ResStationStatusTableEntry **resStationEntriesForInstruction(ResStationStatusTable *resStationTable, Instruction *inst);
int indexForFreeResStation(ResStationStatusTable *resStationTable, int fuType);
int indexForFreeResStationForInstruction(ResStationStatusTable *resStationTable, Instruction *inst);
int isFreeResStationForInstruction(ResStationStatusTable *resStationTable, Instruction *inst);
void addInstToResStation(ResStationStatusTable *resStationTable, RegisterStatusTable *regTable, RegisterFile *regFile, Instruction *inst, int destROB);
void printResStationStatusTable(ResStationStatusTable *resStationTable);
void forwardFloatResultToResStationStatusTable(ResStationStatusTable *resStationTable, FloatFUResult *floatResult);
void setResStationEntryOperandAvailability(ResStationStatusTableEntry *entry, RegisterStatusTable *regTable, RegisterFile *regFile, int sourceNum, int physReg, int resultType);
void processIntForwardingForResStationEntries(ResStationStatusTableEntry **entries, int numEntries, IntFUResult *intResult);
void processFloatForwardingForResStationEntries(ResStationStatusTableEntry **entries, int numEntries, FloatFUResult *floatResult);
void forwardIntResultToResStationStatusTable(ResStationStatusTable *resStationTable, IntFUResult *intResult);