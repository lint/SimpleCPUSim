
enum PhysicalRegisterName; // forward declaration

// struct representing a register file
typedef struct RegisterFile {

    int *data;

} RegisterFile;

// register file methods
void initRegisterFile(RegisterFile *registerFile);
void teardownRegisterFile(RegisterFile *registerFile);
int readRegisterFile(RegisterFile *registerFile, enum PhysicalRegisterName reg);
void writeRegisterFile(RegisterFile *registerFile, enum PhysicalRegisterName reg, int value);