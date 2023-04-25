
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

// just general method which takes a pc and returns the next pc?
// depends on where you want to have the logic
// you could also have a method that determines whether or not the pc has an entry in the BTB and then have the fetch unit call another method which does the lookup?
// probably better to just have the one general method

/*

so how does it work

* fetch unit gets the instruction at the current pc
    * pc val is sent to branch predictor
    * if no entry is found
        * return input PC + 4
        * process is repeated
    * if entry is found
        * use 2-bit dynamic branch predictor to determine whether or not to take the branch
            * if weakly taken or strongly taken, return the target PC
            * if weakly not taken or strongly not taken, return input pc + 4

* how does the instruction know what the target address actually is from the label?

*/
