
// struct which contains both a float and int value to store speculative and real results
typedef struct RegisterFileEntry {
    int intVal;
    float floatVal;
    int valueType; // enum ValueType (only used for printing)
} RegisterFileEntry;

// struct representing the register file
typedef struct RegisterFile {
    RegisterFileEntry **physicalRegisters;
    int numPhysicalRegisters;
    int pc;
} RegisterFile;

// register file methods
void initRegisterFile(RegisterFile *registerFile);
void teardownRegisterFile(RegisterFile *registerFile);
void printRegisterFile(RegisterFile *registerFile);
int readRegisterFileInt(RegisterFile *registerFile, int reg); // reg = enum IntRegisterName
float readRegisterFileFloat(RegisterFile *registerFile, int reg); // reg = enum FloatRegisterName
void writeRegisterFileInt(RegisterFile *registerFile, int reg, int value); // reg = enum IntRegisterName
void writeRegisterFileFloat(RegisterFile *registerFile, int reg, float value); // reg = enum FloatRegisterName
