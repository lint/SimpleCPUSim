
// forward declarations
typedef struct ROBStatusTable ROBStatusTable;
typedef struct ResStationStatusTable ResStationStatusTable;
typedef struct RegisterStatusTable RegisterStatusTable;

// struct that bundles the different status tables together
typedef struct StatusTables {
    ROBStatusTable *robTable;
    ResStationStatusTable *resStationTable;
    RegisterStatusTable *regTable;
} StatusTables;