
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
    int justGotOperandFromCDB; // boolean used to indiciate if a value was received from the CDB so it doesn't get used in the same cycle
    int qj; // ROB index containing source operand 1
    int qk; // ROB index containing source operand 2
    int dest; // the ROB index that will hold the result
    int addr; // store base address information for loads/stores/branches
    int buOffset; // the offset stored in the BU instruction in order to calculate the effective address

} ResStationStatusTableEntry;

// struct representing the reservation status table
typedef struct ResStationStatusTable {

    // have separate arrays for each functional unit (including 2 for the load/store unit)
    ResStationStatusTableEntry **intEntries;
    ResStationStatusTableEntry **loadEntries;
    ResStationStatusTableEntry **storeEntries;
    ResStationStatusTableEntry **fpAddEntries;
    ResStationStatusTableEntry **fpMulEntries;
    ResStationStatusTableEntry **fpDivEntries;
    ResStationStatusTableEntry **buEntries;
    
    int numIntStations;
    int numLoadStations;
    int numStoreStations;
    int numFPAddStations;
    int numFPMulStations;
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
void setResStationEntryOperandAvailability(ResStationStatusTableEntry *entry, RegisterStatusTable *regTable, RegisterFile *regFile, 
    int sourceNum, ArchRegister *reg, int renamedReg, int resultType);
void processIntUpdateForResStationEntries(ResStationStatusTableEntry **entries, int numEntries, int destROB, int result, int fromCDB);
void processFloatUpdateForResStationEntries(ResStationStatusTableEntry **entries, int numEntries, int destROB, float result, int fromCDB);
void sendIntUpdateToResStationStatusTable(ResStationStatusTable *resStationTable, int destROB, int result, int fromCDB);
void sendFloatUpdateToResStationStatusTable(ResStationStatusTable *resStationTable, int destROB, float result, int fromCDB);
void flushResStationStatusTable(ResStationStatusTable *resStationTable);