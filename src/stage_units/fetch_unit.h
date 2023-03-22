
typedef struct Instruction Instruction; // forward declaration

typedef struct FetchUnit {

    Instruction **outputBuffer;
    int outputBufferSize;
    int instsInBuffer;
    int NF;

} FetchUnit;

void initFetchUnit(FetchUnit *fetchUnit, int NF);