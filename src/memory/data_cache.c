
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../misc/misc.h"
#include "data_cache.h"

// initialize a struct representing the data cache
void initDataCache(DataCache *dataCache) {
    printf_DEBUG(("initalizing data cache...\n"));

    dataCache->cacheSize = DATA_CACHE_INITIAL_SIZE;
    dataCache->cache = calloc(DATA_CACHE_INITIAL_SIZE, sizeof(float));
}

// free any elements of the data cache stored on the heap
void teardownDataCache(DataCache *dataCache) {
    if (dataCache->cache) {
        free(dataCache->cache);
    }
}

// checks if a given address is outside the bounds of the data cache and increases its size if so
void extendDataCacheIfNeeded(DataCache *dataCache, int address) {
    
    if (address >= dataCache->cacheSize) {

        int oldSize = dataCache->cacheSize;
        float *oldCache = dataCache->cache;

        // double size of data cache until it can fit the requested address
        int newSize = oldSize * 2;
        while (newSize <= address) {
            newSize *= 2;
        }

        // set the new size and reallocate the data cache
        dataCache->cacheSize = newSize;
        dataCache->cache = calloc(newSize, sizeof(float));
        memcpy(dataCache->cache, oldCache, oldSize);
        free(oldCache);
        
        #ifdef ENABLE_DEBUG_LOG
        printf("address '%d' is greater than the current data cache size '%d', extending to %d entries\n", address, oldSize, newSize);
        #endif
    }
}

// write a float to the given address in the data cache
void writeFloatToDataCache(DataCache *dataCache, int address, float value) {

    if (address < 0) {
        printf_DEBUG(("error: can't write to an address less than 0\n"));
        return;
    }

    // check if requested address is outside current cache size
    extendDataCacheIfNeeded(dataCache, address);

    // write the value to the entry
    dataCache->cache[address] = value;
}

// retrieves a float from a certain address in the data cache
float readFloatFromDataCache(DataCache *dataCache, int address) {

    if (address < 0) {
        printf_DEBUG(("error: can't read from an address less than 0\n"));
        return 0;
    }

    // check if requested address is outside current cache size
    extendDataCacheIfNeeded(dataCache, address);

    // read the float from the cache and return it
    return dataCache->cache[address];
}

// prints the contents of the data cache to the command line
void printDataCache(DataCache *dataCache) {

    printf("data cache: \n");

    // number of bytes to include for each row
    int entriesPerRow = 8;

    // print byte row index
    printf("%7s | ", "");
    for (int i = 0; i < entriesPerRow; i++) {
        printf("%7d", i);
    }
    printf("\n");

    // print separator
    printf("---");
    for (int i = 0; i < (entriesPerRow + 1) * 7; i++) {
        printf("-");
    }
    printf("\n");

    // print each full row of bytes
    for (int i = 0; i < dataCache->cacheSize / entriesPerRow; i++) {
        printf("%7d | ", i * entriesPerRow);
        for (int j = 0; j < entriesPerRow; j++) {
            printf("%7.1f", readFloatFromDataCache(dataCache, i * entriesPerRow + j));
        }
        printf("\n");
    }

    // print the remaining row if the cache cannot be divided evenly
    if (dataCache->cacheSize % entriesPerRow != 0) {
        int lastRow = dataCache->cacheSize / entriesPerRow;

        printf("%7d", lastRow * entriesPerRow);
        for (int j = 0; j < dataCache->cacheSize % entriesPerRow; j++) {
            printf("%7.1f", readFloatFromDataCache(dataCache, lastRow * entriesPerRow + j));
        }
        printf("\n");
    }
}