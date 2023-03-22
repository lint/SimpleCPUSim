
enum PhysicalRegisterName; // forward declaration

// struct representing a register file
typedef struct RegisterFile {

    int *data;
    int pc;

} RegisterFile;

// register file methods
void initRegisterFile(RegisterFile *registerFile);
void teardownRegisterFile(RegisterFile *registerFile);
int readRegisterFile(RegisterFile *registerFile, enum PhysicalRegisterName reg);
void writeRegisterFile(RegisterFile *registerFile, enum PhysicalRegisterName reg, int value);
void printRegisterFile(RegisterFile *registerFile);