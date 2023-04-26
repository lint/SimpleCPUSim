
// struct representing an entry in the BTB
typedef struct BTBEntry {
    int pc;
    int target;
} BTBEntry;

// struct representing the branch predictor
typedef struct BranchPredictor {
    int state; // enum BranchPredictionState
    int numBTBEntries;
    BTBEntry **btb;
} BranchPredictor;

// branch predictor methods
void initBranchPredictor(BranchPredictor *branchPredictor);
void teardownBranchPredictor(BranchPredictor *branchPredictor);
void updateBranchPredictor(BranchPredictor *branchPredictor, int branchWasTaken);
int shouldTakeBranch(BranchPredictor *branchPredictor);
void printBranchPredictor(BranchPredictor *branchPredictor);
int predictNextPC(BranchPredictor *branchPredictor, int pc);
void updateBTBEntry(BranchPredictor *branchPredictor, int pc, int target);