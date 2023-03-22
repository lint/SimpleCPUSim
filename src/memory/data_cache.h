
#define DATA_CACHE_INITIAL_SIZE 256 // initial number of bytes to allocate for the data cache (can be reallocated later if needed)

// struct representing a data cache
typedef struct DataCache {
    int cacheSize;
    char *cache;
} DataCache;

// data cache methods
void initDataCache(DataCache *dataCache);
void teardownDataCache(DataCache *dataCache);
void extendDataCacheIfNeeded(DataCache *dataCache, int address);
void writeByteToDataCache(DataCache *dataCache, int address, char byte);
char readByteFromDataCache(DataCache *dataCache, int address);
void printDataCache(DataCache *dataCache);
