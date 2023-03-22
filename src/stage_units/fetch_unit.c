
#include <stdlib.h>
#include "fetch_unit.h"
#include "../cpu.h"

void initFetchUnit(FetchUnit *fetchUnit, int NF) {

    printf("initializing fetch unit...\n");

    fetchUnit->NF = NF;
    fetchUnit->outputBuffer = calloc(sizeof(Instruction *), NF);
    fetchUnit->outputBufferSize = NF;
    fetchUnit->instsInBuffer = 0;
}

// doubles the size of the output buffer if necessary
void extendOutputBufferIfNeeded(FetchUnit *fetchUnit) {

    // only extend the buffer if the next fetch would fill it
    if (fetchUnit->instsInBuffer + fetchUnit->NF > fetchUnit->outputBufferSize) {
        
        // get old values
        Instruction **oldBuffer = fetchUnit->outputBuffer;
        int oldSize = fetchUnit->outputBufferSize;
        int newSize = oldSize * 2;

        // reallocate array and copy contents
        fetchUnit->outputBufferSize = newSize;
        fetchUnit->outputBuffer = calloc(newSize, sizeof(Instruction *));
        memcpy(fetchUnit->outputBuffer, oldBuffer, oldSize * sizeof(Instruction *));
        free(oldBuffer);

        printf("extending instruction cache to %i entries\n", newSize);
    }
}

// void cycleFetchUnit(FetchUnit *fetchUnit, int pc, )