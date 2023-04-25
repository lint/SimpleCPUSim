
// forward declarations
typedef struct RegisterFile RegisterFile;
typedef struct InstCache InstCache;
typedef struct BranchPredictor BranchPredictor;
typedef struct FetchBufferEntry FetchBufferEntry;

// struct representing a fetch unit
typedef struct FetchUnit {
    // char **fetchBuffer;
    FetchBufferEntry **fetchBuffer;
    int *fetchBufferSize;
    int *numInstsInBuffer;
    int NF;
} FetchUnit;

// fetch unit methods
void initFetchUnit(FetchUnit *fetchUnit, int NF, FetchBufferEntry **fetchBuffer, int *fetchBufferSize, int *numInstsInBuffer);
void teardownFetchUnit(FetchUnit *fetchUnit);
void addInstToFetchUnitOutputBuffer(FetchUnit *fetchUnit, char *instStr, int instAddr);
void extendFetchUnitOutputBufferIfNeeded(FetchUnit *fetchUnit);
void cycleFetchUnit(FetchUnit *fetchUnit, RegisterFile *registerFile, InstCache *instCache, BranchPredictor *branchPredictor);