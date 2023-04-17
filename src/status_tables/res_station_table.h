
// forward declarations
typedef struct RegisterFile RegisterFile;
typedef struct RegisterStatusTable RegisterStatusTable;

// struct representing the reservation status table's entries
typedef struct ResStationStatusTableEntry {
    int fuType; // enum FunctionalUnitType
    int resStationIndex;
    int busy;
    int op; // enum FunctionalUnitOperation
    // variable that keeps track of whether or not the operands are ints or floats?
    // or just do this with functional unit type?
    int vjInt;
    int vkInt;
    float vjFloat;
    float vkFloat;
    int vjIsAvailable;
    int vkIsAvailable;
    int qj; // ROB index containing source operand 1 (are these ones that are currently in the ROB or that will be available in the ROB?)
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
void initResStationStatusTable(ResStationStatusTable *resStationTable);
void teardownResStationStatusTable(ResStationStatusTable *resStationTable);
int indexForFreeResStation(ResStationStatusTable *resStationTable, enum FunctionalUnitType fuType);
int indexForFreeResStationForInstruction(ResStationStatusTable *resStationTable, Instruction *inst);
int isFreeResStationForInstruction(ResStationStatusTable *resStationTable, Instruction *inst);
void addInstToResStation(ResStationStatusTable *resStationTable, RegisterStatusTable *regTable, RegisterFile *regFile, Instruction *inst, int destROB);
void printResStationStatusTable(ResStationStatusTable *resStationTable);