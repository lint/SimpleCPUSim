
#define DATA_CACHE_INITIAL_SIZE 256 // initial number of bytes to allocate for the data cache (can be reallocated later if needed)

// struct representing a data cache
typedef struct DataCache {
    int cacheSize;
    float *cache;
} DataCache;

// data cache methods
void initDataCache(DataCache *dataCache);
void teardownDataCache(DataCache *dataCache);
void extendDataCacheIfNeeded(DataCache *dataCache, int address);
void writeFloatToDataCache(DataCache *dataCache, int address, float value);
float readFloatFromDataCache(DataCache *dataCache, int address);
void printDataCache(DataCache *dataCache);
