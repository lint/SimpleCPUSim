
// struct containing information for an INT functional unit operation
typedef struct IntFUResult {
    int source1;
    int source2;
    int result;
    int destROB;
} IntFUResult;

// struct containing information for a FPAdd, FPMult, or FPDiv functional unit operation
typedef struct FloatFUResult {
    float source1;
    float source2;
    float result;
    int destROB;
} FloatFUResult;