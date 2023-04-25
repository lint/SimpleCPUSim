
// forward declarations
typedef struct IntFunctionalUnit IntFunctionalUnit;
typedef struct FPFunctionalUnit FPFunctionalUnit;
typedef struct BUFunctionalUnit BUFunctionalUnit;
typedef struct LSFunctionalUnit LSFunctionalUnit;

// struct containing information for an INT functional unit operation
typedef struct IntFUResult {
    int source1;
    int source2;
    int result;
    int destROB;
} IntFUResult;

// struct containing information for a FPAdd, FPMul, or FPDiv functional unit operation
typedef struct FloatFUResult {
    float source1;
    float source2;
    float result;
    int destROB;
} FloatFUResult;

// struct containing infromation for a BU functional unit operation
typedef struct BUFUResult {
    int source1;
    int source2;
    int isBranchTaken;
    int effAddr;
    int destROB;
} BUFUResult;

// struct containing information for the load/store functional unit operation
typedef struct LSFUResult {
    int offset;
    int base;
    int resultAddr;
    int destROB;
    int fuType; // either FU_TYPE_LOAD or FU_TYPE_STORE
    float loadValue; // used in the memory unit
} LSFUResult;

// struct that bundles the functional units together 
typedef struct FunctionalUnits {
    IntFunctionalUnit *intFU;
    FPFunctionalUnit *fpAddFU;
    FPFunctionalUnit *fpMulFU;
    FPFunctionalUnit *fpDivFU;
    BUFunctionalUnit *buFU;
    LSFunctionalUnit *lsFU;

} FunctionalUnits;