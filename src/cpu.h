
// forward declarations
typedef struct Params Params; 
typedef struct InstCache InstCache;
typedef struct DataCache DataCache;
typedef struct RegisterFile RegisterFile;
typedef struct FetchUnit FetchUnit;
typedef struct DecodeUnit DecodeUnit;

// struct representing the CPU
typedef struct CPU {

    Params *params;
    InstCache *instCache;
    DataCache *dataCache;
    RegisterFile *registerFile;

    FetchUnit *fetchUnit;
    DecodeUnit *decodeUnit;

} CPU;

void initCPU(CPU *cpu, Params *params);
void teardownCPU(CPU *cpu);
void executeCPU(CPU *cpu);