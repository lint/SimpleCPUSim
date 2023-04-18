
// forward declarations
enum PhysicalRegisterName; // TODO: remove this?
typedef struct DecodeUnit DecodeUnit;

typedef struct RegisterFileEntry {
    int intVal;
    float floatVal;
    int valType; // enum InstructionResultValueType
} RegisterFileEntry;

// struct representing a register file
typedef struct RegisterFile {

    RegisterFileEntry **entries;
    int numEntries;
    int pc;

} RegisterFile;

// register file methods
void initRegisterFile(RegisterFile *registerFile);
void teardownRegisterFile(RegisterFile *registerFile);
int readRegisterFileInt(RegisterFile *registerFile, enum PhysicalRegisterName reg);
float readRegisterFileFloat(RegisterFile *registerFile, enum PhysicalRegisterName reg);
void writeRegisterFileInt(RegisterFile *registerFile, enum PhysicalRegisterName reg, int value);
void writeRegisterFileFloat(RegisterFile *registerFile, enum PhysicalRegisterName reg, float value);
void printRegisterFile(RegisterFile *registerFile);
void printRegisterFileArchRegs(RegisterFile *registerFile, DecodeUnit *decodeUnit);