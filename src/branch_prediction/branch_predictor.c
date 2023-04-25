
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../misc/misc.h"
#include "branch_predictor.h"

// initialize a branch predictor struct
void initBranchPredictor(BranchPredictor *branchPredictor) {
    
    // values are specified in the project description
    branchPredictor->state = BRANCH_STATE_WEAKLY_TAKEN; 
    branchPredictor->numBTBEntries = 16;
    
    branchPredictor->btb = malloc(branchPredictor->numBTBEntries * sizeof(BTBEntry *));

    for (int i = 0; i < branchPredictor->numBTBEntries; i++) {
        BTBEntry *entry = malloc(sizeof(BTBEntry));
        entry->pc = -1;
        entry->target = -1;

        branchPredictor->btb[i] = entry;
    }
}

// free any elements of the branch predictor that are stored on the heap
void teardownBranchPredictor(BranchPredictor *branchPredictor) {

    if (branchPredictor->btb) {
        for (int i = 0; i < branchPredictor->numBTBEntries; i++) {
            free(branchPredictor->btb[i]);
        }
        free(branchPredictor->btb);
    }
}

// updates the state of the branch predictor depending on if a branch was taken or not
void updateBranchPredictor(BranchPredictor *branchPredictor, int branchWasCorrect) {

    printf("updating branch predictor state from: %s ", branchPredictionStateToString(branchPredictor->state));
    
    // branch was predicted correctly
    if (branchWasCorrect) {

        // // strongly not taken -> weakly not taken
        // if (branchPredictor->state == BRANCH_STATE_STRONGLY_NOT_TAKEN) {
        //     branchPredictor->state = BRANCH_STATE_WEAKLY_NOT_TAKEN;
        
        // // weakly not taken -> weakly taken
        // } else if (branchPredictor->state == BRANCH_STATE_WEAKLY_NOT_TAKEN) {
        //     branchPredictor->state = BRANCH_STATE_WEAKLY_TAKEN;
        
        // // weakly taken -> strongly taken
        // } else if (branchPredictor->state == BRANCH_STATE_WEAKLY_TAKEN) {
        //     branchPredictor->state = BRANCH_STATE_STRONGLY_TAKEN;
        // }

        // weakly not taken -> strongly not taken
        if (branchPredictor->state == BRANCH_STATE_WEAKLY_NOT_TAKEN) {
            branchPredictor->state = BRANCH_STATE_STRONGLY_NOT_TAKEN;
        
        // weakly taken -> strongly taken
        } else if (branchPredictor->state == BRANCH_STATE_WEAKLY_TAKEN) {
            branchPredictor->state = BRANCH_STATE_STRONGLY_TAKEN;
        }
    
    // branch was not predicted correctly
    } else {

        // // strongly taken -> weakly taken
        // if (branchPredictor->state == BRANCH_STATE_STRONGLY_TAKEN) {
        //     branchPredictor->state = BRANCH_STATE_WEAKLY_TAKEN;
        
        // // weakly taken -> weakly not taken
        // } else if (branchPredictor->state == BRANCH_STATE_WEAKLY_TAKEN) {
        //     branchPredictor->state = BRANCH_STATE_WEAKLY_NOT_TAKEN;
        
        // // weakly not taken -> strongly not taken
        // } else if (branchPredictor->state == BRANCH_STATE_WEAKLY_NOT_TAKEN) {
        //     branchPredictor->state = BRANCH_STATE_STRONGLY_NOT_TAKEN;
        // }

        // strongly taken -> weakly taken
        if (branchPredictor->state == BRANCH_STATE_STRONGLY_TAKEN) {
            branchPredictor->state = BRANCH_STATE_WEAKLY_TAKEN;

        // weakly taken -> weakly not taken
        } else if (branchPredictor->state == BRANCH_STATE_WEAKLY_TAKEN) {
            branchPredictor->state = BRANCH_STATE_WEAKLY_NOT_TAKEN;

        // weakly not taken -> weakly taken
        } else if (branchPredictor->state == BRANCH_STATE_WEAKLY_NOT_TAKEN) {
            branchPredictor->state = BRANCH_STATE_WEAKLY_TAKEN;

        // strongly not taken -> weakly not taken
        } else if (branchPredictor->state == BRANCH_STATE_STRONGLY_NOT_TAKEN) {
            branchPredictor->state = BRANCH_STATE_WEAKLY_NOT_TAKEN;
        }
    }

    printf("to: %s\n", branchPredictionStateToString(branchPredictor->state));
}

// returns the BTB that a given PC hashes to
BTBEntry *getBTBEntryForPC(BranchPredictor *branchPredictor, int pc) {

    // use bits 7-4 of the pc to index into the BTB
    int mask = 0xF0; // 0b11110000 
    int index = (pc & mask) >> 4;

    return branchPredictor->btb[index];
}

// returns whether or not the branch predictor should take the branch
int shouldTakeBranch(BranchPredictor *branchPredictor) {
    
    if (branchPredictor->state == BRANCH_STATE_STRONGLY_TAKEN || branchPredictor->state == BRANCH_STATE_WEAKLY_TAKEN) {
        return 1;
    } else if (branchPredictor->state == BRANCH_STATE_STRONGLY_NOT_TAKEN || branchPredictor->state == BRANCH_STATE_WEAKLY_NOT_TAKEN) {
        return 0;
    } else {
        printf("error: branch predictor is in invalid state\n");
        return -1;
    }
}

// updates the BTB for a given address and target
void updateBTBEntry(BranchPredictor *branchPredictor, int pc, int target) {

    BTBEntry *entry = getBTBEntryForPC(branchPredictor, pc);
    entry->pc = pc;
    entry->target = target;
}

// predict the next PC for a given PC
int predictNextPC(BranchPredictor *branchPredictor, int pc) {

    printf("branch predictor getting next pc for given pc: %i\n", pc);

    // get the entry associated with the given pc
    BTBEntry *entry = getBTBEntryForPC(branchPredictor, pc);


    // check if entry matches the given pc
    if (pc == entry->pc) {

        // use 2-bit dynamic state to decide to take the branch or not
        if (shouldTakeBranch(branchPredictor)) {
            printf("\tentry matches provided pc and predicted to take branch, returning new pc: %i\n", entry->target);
            return entry->target;
        } else {
            printf("\tentry matches provided pc and predicted to take not branch, returning new pc: %i\n", pc + 4);
            return pc + 4;
        }
    
    // entry does not match the given pc (it's either a different branch or the entry has not been used before)
    } else {
        printf("\tentry does not match provided pc, returning new pc: %i\n", pc + 4);
        return pc + 4;
    }
}

// prints the contents of the branch predictor
void printBranchPredictor(BranchPredictor *branchPredictor) {
    
    printf("branch predictor: state: %s\n", branchPredictionStateToString(branchPredictor->state));

    for (int i = 0; i < branchPredictor->numBTBEntries; i++) {
        BTBEntry *entry = branchPredictor->btb[i];
        printf("\tBTB entry: %i, pc: %i, target: %i\n", i, entry->pc, entry->target);
    }
}