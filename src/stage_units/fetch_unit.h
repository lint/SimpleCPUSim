
// forward declarations
typedef struct Instruction Instruction;
typedef struct RegisterFile RegisterFile;
typedef struct InstCache InstCache;

// struct representing a fetch unit
typedef struct FetchUnit {
    Instruction **outputBuffer;
    int outputBufferSize;
    int numInstsInBuffer;
    int NF;
} FetchUnit;

// fetch unit methods
void initFetchUnit(FetchUnit *fetchUnit, int NF);
void teardownFetchUnit(FetchUnit *fetchUnit);
void addInstToFetchUnitOutputBuffer(FetchUnit *fetchUnit, Instruction *inst);
void extendFetchUnitOutputBufferIfNeeded(FetchUnit *fetchUnit);
void printFetchUnitOutputBuffer(FetchUnit *fetchUnit);
void cycleFetchUnit(FetchUnit *fetchUnit, RegisterFile *registerFile, InstCache *instCache);