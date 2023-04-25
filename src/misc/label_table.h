
// struct representing an entry in the label table
typedef struct LabelTableEntry {
    int addr;
    char *label;
    struct LabelTableEntry *next;
} LabelTableEntry;

// struct representing a table which is used to find which labels are associated with which instructions
typedef struct LabelTable {
    LabelTableEntry *labelTableHead;
} LabelTable;

// label table methods
void initLabelTable(LabelTable *labelTable);
void teardownLabelTable(LabelTable *labelTable);
void addEntryToLabelTable(LabelTable *labelTable, int addr, char *label);
int getAddressForLabel(LabelTable *labelTable, char *label);
void printLabelTable(LabelTable *labelTable);