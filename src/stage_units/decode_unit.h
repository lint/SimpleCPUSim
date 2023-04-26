
// forward declarations
typedef struct Instruction Instruction;
typedef struct RegisterMappingNode RegisterMappingNode;
typedef struct StatusTables StatusTables;
typedef struct RegisterFile RegisterFile;
typedef struct StallStats StallStats;
typedef struct ArchRegister ArchRegister;
typedef struct LabelTable LabelTable;
typedef struct FetchBufferEntry FetchBufferEntry;
typedef struct ROBStatusTable ROBStatusTable;

// struct representing a node storing register information for the mapping table and free list
typedef struct RegisterMappingNode {
    int reg; // enum PhysicalRegisterName
    struct RegisterMappingNode *next;
} RegisterMappingNode;

// struct representing an entry in the map table
typedef struct MapTableEntry {
    ArchRegister *reg;
    RegisterMappingNode *mapHead;
    struct MapTableEntry *next;
} MapTableEntry;

// struct representing a decode unit
typedef struct DecodeUnit {
    Instruction **decodeQueue;
    int numInstsInQueue;
    int NI;
    int NW;
    RegisterMappingNode *freeList;
    MapTableEntry *mapTableHead;
} DecodeUnit;

// decode unit methods
void initDecodeUnit(DecodeUnit *decodeUnit, int NI, int NW);
void teardownDecodeUnit(DecodeUnit *decodeUnit);
void addInstructionToDecodeQueue(DecodeUnit *decodeUnit, LabelTable *labelTable, char *instStr, int instAddr);
void printMapTable(DecodeUnit *decodeUnit);
void printFreeList(DecodeUnit *decodeUnit);
void printDecodeQueue(DecodeUnit *decodeUnit);
int numFreePhysicalRegistersAvailable(DecodeUnit *decodeUnit);
RegisterMappingNode *getFreePhysicalRegister(DecodeUnit *decodeUnit);
MapTableEntry *mapTableEntryForRegister(DecodeUnit *decodeUnit, ArchRegister *reg);
int physicalRegisterMappingForReg(DecodeUnit *decodeUnit, ArchRegister *reg); //returns PhysicalRegisterName
void addPhysicalRegisterToMapTable(DecodeUnit *decodeUnit, RegisterMappingNode *physRegNode, ArchRegister *reg);
int readMapTableForReg(DecodeUnit *decodeUnit, ArchRegister *reg); // returns PhysicalRegisterName
int numPhysicalRegisterMappingsForReg(DecodeUnit *decodeUnit, ArchRegister *reg);
RegisterMappingNode *getFreeListTailNode(DecodeUnit *decodeUnit);
void popOldPhysicalRegisterMappingForReg(DecodeUnit *decodeUnit, ROBStatusTable *robTable, ArchRegister *reg, int addToTail);
void popNewPhysicalRegisterMappingForReg(DecodeUnit *decodeUnit, ArchRegister *reg, int addToTail);
int numPhysicalRegistersNeededForInst(DecodeUnit *decodeUnit, Instruction *inst);
int performRegisterRenamingForInst(DecodeUnit *decodeUnit, Instruction *inst);
void cycleDecodeUnit(DecodeUnit *decodeUnit, FetchBufferEntry **fetchBuffer, int *numInstsInBuffer, StatusTables *statusTables, RegisterFile *regFile, StallStats *stallStats, LabelTable *labelTable);
void flushDecodeQueue(DecodeUnit *decodeUnit);