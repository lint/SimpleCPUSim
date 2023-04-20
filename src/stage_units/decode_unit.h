
// forward declarations
typedef struct Instruction Instruction;
typedef struct RegisterMappingNode RegisterMappingNode;
typedef struct StatusTables StatusTables;
typedef struct RegisterFile RegisterFile;
typedef struct StallStats StallStats;
typedef struct ArchRegister ArchRegister;

// struct representing an entry in the map table
typedef struct MapTableEntry {
    ArchRegister *reg;
    RegisterMappingNode *mapHead;
    struct MapTableEntry *next;
} MapTableEntry;

// struct representing a decode unit
typedef struct DecodeUnit {
    Instruction **decodeQueue;
    Instruction **fetchBuffer;
    int *fetchBufferSize;
    int *numInstsInBuffer;
    int numInstsInQueue;
    int NI;
    int NW;

    // RegisterMappingNode **intRegMapTable;
    // RegisterMappingNode **floatRegMapTable;
    RegisterMappingNode *freeList;
    MapTableEntry *mapTableHead;
} DecodeUnit;

// decode unit methods
void initDecodeUnit(DecodeUnit *decodeUnit, int NF, int NI, Instruction **fetchBuffer, int *fetchBufferSize, int *numInstsInBuffer);
void teardownDecodeUnit(DecodeUnit *decodeUnit);
// void extendDecodeUnitInputBufferIfNeeded(DecodeUnit *decodeUnit);
// void setDecodeUnitInputBufferSize(DecodeUnit *decodeUnit, int newSize);
void addInstructionToDecodeQueue(DecodeUnit *decodeUnit, Instruction *inst);
void printMapTable(DecodeUnit *decodeUnit);
void printFreeList(DecodeUnit *decodeUnit);
void printDecodeQueue(DecodeUnit *decodeUnit);
// void printDecodeUnitInputBuffer(DecodeUnit *decodeUnit);
// void printDecodeUnitOutputQueue(DecodeUnit *decodeUnit);
//RegisterMappingNode *getFreePhysicalRegister(DecodeUnit *decodeUnit);
RegisterMappingNode *getFreeRenameRegister(DecodeUnit *decodeUnit);
RegisterMappingNode *getFreeListTailNode(DecodeUnit *decodeUnit);
void popOldPhysicalRegisterMappingForReg(DecodeUnit *decodeUnit, ArchRegister *reg);
//void addPhysicalRegisterToMapTable(DecodeUnit *decodeUnit, RegisterMappingNode *physRegNode, enum RegisterName reg);
// void addRenameRegisterToIntMapTable(DecodeUnit *decodeUnit, RegisterMappingNode *renameRegNode, int reg); // reg = enum IntRegisterName
// void addRenameRegisterToFloatMapTable(DecodeUnit *decodeUnit, RegisterMappingNode *renameRegNode, int reg); // reg = enum FloatRegisterName
// enum PhysicalRegisterName readMapTable(DecodeUnit *decodeUnit, enum RegisterName reg);
enum RenameRegisterName readIntMapTableForReg(DecodeUnit *decodeUnit, int reg); // reg = enum IntRegisterName
enum RenameRegisterName readFloatMapTableForReg(DecodeUnit *decodeUnit, int reg); // reg = enum FloatRegisterName
void performRegisterRenaming(DecodeUnit *decodeUnit, Instruction *inst);
void cycleDecodeUnit(DecodeUnit *decodeUnit, StatusTables *statusTables, RegisterFile *regFile, StallStats *stallStats);

int numFreeRenameRegistersAvailable(DecodeUnit *decodeUnit);

// TODO remove these methods and add the current ones..
