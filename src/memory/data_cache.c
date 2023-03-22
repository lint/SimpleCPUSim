
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data_cache.h"

// initialize a struct representing the data cache
void initDataCache(DataCache *dataCache) {
    printf("initalizing data cache...\n");
    dataCache->cacheSize = DATA_CACHE_INITIAL_SIZE;
    dataCache->cache = calloc(DATA_CACHE_INITIAL_SIZE, 1);
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
        char *oldCache = dataCache->cache;

        // double size of data cache until it can fit the requested address
        int newSize = oldSize * 2;
        while (newSize <= address) {
            newSize *= 2;
        }

        // set the new size and reallocate the data cache
        dataCache->cacheSize = newSize;
        dataCache->cache = calloc(newSize, 1);
        memcpy(dataCache->cache, oldCache, oldSize);
        free(oldCache);

        printf("address '%d' is greater than the current data cache size '%d', extending to %d bytes\n", address, oldSize, newSize);
    }
}

// writes a byte to a certain address in the data cache
void writeByteToDataCache(DataCache *dataCache, int address, char byte) {

    if (address < 0) {
        printf("error: can't write to an address less than 0\n");
        return;
    }

    // check if requested address is outside current cache size
    extendDataCacheIfNeeded(dataCache, address);

    // write the byte to the cache
    dataCache->cache[address] = byte;
}

// retrieves a byte from a certain address in the data cache
char readByteFromDataCache(DataCache *dataCache, int address) {

    if (address < 0) {
        printf("error: can't read from an address less than 0\n");
        return '\0';
    }

    // check if requested address is outside current cache size
    extendDataCacheIfNeeded(dataCache, address);

    // read the byte from the cache and return it
    return dataCache->cache[address];
}

// prints the contents of the data cache to the command line
void printDataCache(DataCache *dataCache) {

    // number of bytes to include for each row
    int entriesPerRow = 8;

    // print byte row index
    printf("%5s | ", "");
    for (int i = 0; i < entriesPerRow; i++) {
        printf("%5d", i);
    }
    printf("\n");

    // print separator
    printf("---");
    for (int i = 0; i < (entriesPerRow + 1) * 5; i++) {
        printf("-");
    }
    printf("\n");

    // print each full row of bytes
    for (int i = 0; i < dataCache->cacheSize / entriesPerRow; i++) {
        //printf("0x%-3x", i);
        printf("%5d | ", i * entriesPerRow);
        for (int j = 0; j < entriesPerRow; j++) {
            printf("%5d", readByteFromDataCache(dataCache, i * entriesPerRow + j));
        }
        printf("\n");
    }

    // print the remaining row if the cache cannot be divided evenly
    if (dataCache->cacheSize % entriesPerRow != 0) {
        int lastRow = dataCache->cacheSize / entriesPerRow;

        //printf("0x%-3x", i);
        printf("%5d", lastRow * entriesPerRow);
        for (int j = 0; j < dataCache->cacheSize % entriesPerRow; j++) {
            printf("%5d", readByteFromDataCache(dataCache, lastRow * entriesPerRow + j));
        }
        printf("\n");
    }
}