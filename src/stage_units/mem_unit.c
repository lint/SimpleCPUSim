
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../memory/memory.h"
#include "../misc/misc.h"
#include "../functional_units/functional_units.h"
#include "../status_tables/status_tables.h"
#include "mem_unit.h"

// initialize memory unit struct
void initMemoryUnit(MemoryUnit *memUnit) {
    memUnit->currResult = NULL;
    memUnit->isStalledFromWB = 0;
    memUnit->isStalledFromStore = 0;
    memUnit->forwardedData = 0;
    memUnit->forwardedAddr = -1;
}

// free any elements of the memory unit that are stored on the heap
void teardownMemoryUnit(MemoryUnit *memUnit) {
    // no elements
}

// flushes any contents of the memory unit
void flushMemUnit(MemoryUnit *memUnit) {
    memUnit->currResult = NULL;
    memUnit->isStalledFromWB = 0;
    memUnit->isStalledFromStore = 0;
    memUnit->forwardedData = 0;
    memUnit->forwardedAddr = -1;
}

// returns the result from the memory unit if it is not stalled while waiting for stores to clear
LSFUResult *getMemoryUnitCurrentResult(MemoryUnit *memUnit) {
    
    // if memory unit was stalled because a store is writing to the same address that the load is reading from
    if (memUnit->isStalledFromStore) {
        return NULL;
    } else {
        return memUnit->currResult;
    }
}

// receive forwarded data from WB unit when a store enters writeback stage
void forwardDataToMemoryUnit(MemoryUnit *memUnit, float value, int addr) {
    #ifdef ENABLE_DEBUG_LOG
    printf("memory unit received forwarded data: %f from address: %i\n", value, addr);
    #endif

    memUnit->forwardedData = value;
    memUnit->forwardedAddr = addr;
}

// perform memory unit operations over a cycle
void cycleMemoryUnit(MemoryUnit *memUnit, DataCache *dataCache, LSFunctionalUnit *lsFU, StatusTables *statusTables) {
    printf_DEBUG(("\nperforming memory unit operations...\n"));

    ROBStatusTable *robTable = statusTables->robTable;

    // check if the writeback unit did not take in result and place it on CDB
    if (memUnit->isStalledFromWB) {
        printf_DEBUG(("\tmemory unit is stalled due to WB unit not placing data on the CDB"));

        clearMemoryUnitForwardedData(memUnit);
        lsFU->isStalled = 1;
        return;
    } else {
        lsFU->isStalled = 0;
    }

    LSFUResult *lsResult = NULL;

    // check if stalled from a store in a previous cycle
    if (memUnit->isStalledFromStore) {
        printf_DEBUG(("memory unit is stalled from store in previous cycle, trying current result again\n"));

        lsResult = memUnit->currResult;
        lsFU->isStalled = 1;
    
    // if not get the result from the load/store functional unit
    } else {
        printf_DEBUG(("memory unit is not stalled from previous store, getting load/store functional unit results\n"));

        lsResult = getCurrentLSFunctionalUnitResult(lsFU);
        memUnit->currResult = lsResult;
        lsFU->isStalled = 0;
    }

    // do not perform operations if there is no result
    if (!lsResult) {
        printf_DEBUG(("there are no load/store results available, returning...\n"));
        clearMemoryUnitForwardedData(memUnit);
        return;
    }

    // check if a load instruction is currently in the memory unit
    if (lsResult->fuType == FU_TYPE_LOAD) {

        // need to check if there is any stores ahead of the load that will write to the given address

        // check if data was forwarded from the WB stage 
        if (memUnit->forwardedAddr == lsResult->resultAddr) {
            lsResult->loadValue = memUnit->forwardedData;

            #ifdef ENABLE_DEBUG_LOG
            printf("load with addr: %i received forwarded float: %f", memUnit->forwardedAddr, memUnit->forwardedData);
            #endif 

        // no data was forwarded, proceed as normal
        } else {
            #ifdef ENABLE_DEBUG_LOG
            printf("result destROB: %i, robTableHeadIndex: %i\n", lsResult->destROB, robTable->headEntryIndex);
            #endif

            if (lsResult->destROB != robTable->headEntryIndex) {

                for (int i = robTable->headEntryIndex; i != lsResult->destROB; i = (i + 1) % robTable->NR) {
                    
                    #ifdef ENABLE_DEBUG_LOG
                    printf("checking ROB: %i for store conflict\n", i);
                    #endif

                    ROBStatusTableEntry *entry = robTable->entries[i];
                    
                    // check if there is a store between the load ROB and the head ROB that writes to the address 
                    if (entry->busy && entry->fuType == FU_TYPE_STORE && entry->addr == lsResult->resultAddr) {
                        printf_DEBUG(("found store which writes to same address as the current load, stalling memory unit and load/store functional unit\n"));

                        memUnit->isStalledFromStore = 1;
                        lsFU->isStalled = 1;

                        clearMemoryUnitForwardedData(memUnit);
                        return;
                    }
                }
            }
            
            // no conflicts were found, can load value from memory at the calculated address
            lsResult->loadValue = readFloatFromDataCache(dataCache, lsResult->resultAddr);

            #ifdef ENABLE_DEBUG_LOG
            printf("memory unit loaded value: %f from the data cache at address: %i!\n", lsResult->loadValue, lsResult->resultAddr);            
            #endif
        }

        // unstall load/store functional unit and memory unit as execution was able to proceed
        lsFU->isStalled = 0;
        memUnit->isStalledFromStore = 0;

        // forward result to any waiting functional units
        sendFloatUpdateToResStationStatusTable(statusTables->resStationTable, lsResult->destROB, lsResult->loadValue, 0);

    } else if (lsResult->fuType == FU_TYPE_STORE) {

        // stores do not do anything in the memory unit, they just get passed on to the WB unit

    } else {
        #ifdef ENABLE_DEBUG_LOG
        printf("error: invalid functional unit (neither load or store) from LSFUResult in memory unit, this shouldn't happen\n");
        #endif
    }

    clearMemoryUnitForwardedData(memUnit);
}

// prints the current contents of the memory unit
void printMemoryUnit(MemoryUnit *memUnit) {
    printf("Memory unit: isStalledFromWB: %i, isStalledFromStore: %i\n", memUnit->isStalledFromWB, memUnit->isStalledFromStore);
    
    LSFUResult *result = memUnit->currResult;
    if (memUnit->currResult) {
        printf("\tcurrResult: entry: %p, base: %i, offset: %i, resultAddr: %i, destROB: %i, fuType: %s\n", 
                result, result->base, result->offset, result->resultAddr, result->destROB, fuTypeToString(result->fuType));
    } else {
        printf("\tcurrResult: NULL\n");
    }
}

// clears the forwardeded data
void clearMemoryUnitForwardedData(MemoryUnit *memUnit) {
    memUnit->forwardedData = 0;
    memUnit->forwardedAddr = -1;
}