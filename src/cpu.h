
// forward declarations
typedef struct Params Params; 
typedef struct InstCache InstCache;
typedef struct DataCache DataCache;
typedef struct RegisterFile RegisterFile;
typedef struct FetchUnit FetchUnit;
typedef struct RegisterMappingNode RegisterMappingNode;

// struct representing the CPU
typedef struct CPU {

    Params *params;
    InstCache *instCache;
    DataCache *dataCache;
    RegisterFile *registerFile;

    FetchUnit *fetchUnit;

    RegisterMappingNode **registerMapTable;
    RegisterMappingNode *registerFreeList;

} CPU;

void initCPU(CPU *cpu, Params *params);
void teardownCPU(CPU *cpu);
void executeCPU(CPU *cpu);