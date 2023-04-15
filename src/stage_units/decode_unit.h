
// forward declarations
typedef struct Instruction Instruction;
typedef struct RegisterMappingNode RegisterMappingNode;
enum RegisterName;

// struct representing a decode unit
typedef struct DecodeUnit {
    Instruction **instDecodeQueue;
    Instruction **instFetchBuffer;
    int *instFetchBufferSize;
    int *numInstsInBuffer;
    int *numInstsInQueue;
    int NI;
    RegisterMappingNode **registerMapTable;
    RegisterMappingNode *registerFreeList;
} DecodeUnit;

// decode unit methods
void initDecodeUnit(DecodeUnit *decodeUnit, int NF, int NI, 
    Instruction **instFetchBuffer, int *instFetchBufferSize, int *numInstsInBuffer, 
    Instruction **instDecodeQueue, int *numInstsInQueue);
void teardownDecodeUnit(DecodeUnit *decodeUnit);
// void extendDecodeUnitInputBufferIfNeeded(DecodeUnit *decodeUnit);
// void setDecodeUnitInputBufferSize(DecodeUnit *decodeUnit, int newSize);
void addInstructionToDecodeQueue(DecodeUnit *decodeUnit, Instruction *inst);
void printMapTable(DecodeUnit *decodeUnit);
void printFreeList(DecodeUnit *decodeUnit);
// void printDecodeUnitInputBuffer(DecodeUnit *decodeUnit);
// void printDecodeUnitOutputQueue(DecodeUnit *decodeUnit);
RegisterMappingNode *getFreePhysicalRegister(DecodeUnit *decodeUnit);
void addPhysicalRegisterToMapTable(DecodeUnit *decodeUnit, RegisterMappingNode *physRegNode, enum RegisterName reg);
enum PhysicalRegisterName readMapTable(DecodeUnit *decodeUnit, enum RegisterName reg);
void performRegisterRenaming(DecodeUnit *decodeUnit, Instruction *inst);
void cycleDecodeUnit(DecodeUnit *decodeUnit);
