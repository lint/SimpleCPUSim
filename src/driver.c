#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "cpu.h"
#include "misc/misc.h"
#include "memory/memory.h"

// populates the params struct with values from a config file
void readConfig(char *configFn, Params *params) {

    printf("\nprocessing config file...\n");

    // setup default parameter values
    params->NF = 4;
    params->NI = 16;
    params->NW = 4;
    params->NR = 16;
    params->NB = 4;

    FILE *fp = fopen(configFn, "r");
    if (fp == NULL) {
        printf("error: could not open config file, using all default values\n");
    } else {
        
        char key[128];
        int value;
        
        // read the file looking for inputs in the form of 'parameter value'
        // note this fails on malformed input files such as containing a line NF 1 NI, NI also gets assigned 1 
        while (fscanf(fp, "%s %d", key, &value) != EOF) {
            // printf("key: %s, value: %d\n", key, value);

            if (value < 1) {
                printf("found invalid value '%d' for key '%s' when reading config, skipping...\n", value, key);
                continue;
            }

            // check each key and see if it's a parameter
            if (!strcmp(key, "NF")) {
                params->NF = value;
            } else if (!strcmp(key, "NI")) {
                params->NI = value;
            } else if (!strcmp(key, "NW")) {
                params->NW = value;
            } else if (!strcmp(key, "NR")) {
                params->NR = value;
            } else if (!strcmp(key, "NB")) {
                params->NB = value;
            } else {
                printf("found invalid key '%s', skipping...\n", key);
            }
        }

        fclose(fp);
    }

    printf("parameters:\n\tNF = %d\n\tNI = %d\n\tNW = %d\n\tNR = %d\n\tNB = %d\n", params->NF, params->NI, params->NW, params->NR, params->NB);
}

// process input file
void processInput(char *inputFn, CPU *cpu) {

    printf("\nprocessing input program file...\n");
    
    FILE *fp = fopen(inputFn, "r");
    if (fp == NULL) {
        printf("error: could not open input file, exiting...\n");
        exit(1);
    }

    int bufSize = 1024;
    char buf[bufSize];
    int isProcessingInsts = 0;
    int readError = 0;
    int numInsts = 0;

    // read input file line by line
    while (fgets(buf, bufSize, fp) != NULL) {
        // printf("line: %s\n", buf);

        // skip comment lines that start with %
        if (buf[0] == '%') {
            // printf("read comment, skipping...\n");
            continue;
        }

        // replace % with null character to remove any comments in the middle of the line
        for (int i = 0; i < bufSize && buf[i] != '\0'; i++) {
            if (buf[i] == '%') {
                buf[i] = '\0';
                break;
            }
        }
        
        // switch from processing memory content (data) to processing instructions (text) after encountering the first empty line
        if (buf[0] == '\n' || buf[0] == '\r') {
            // printf("found empty line, switching to processing instructions mode\n");
            isProcessingInsts = 1;
            continue;
        }

        // process memory contents
        if (!isProcessingInsts) {

            // read the address and associated value from the current line
            int address, value;
            int matched = sscanf(buf, "%d, %d", &address, &value);

            // check for errors
            if (matched != 2) {
                printf("error: did not match 'address, value' in line: '%s'\n", buf);
                readError = 1;
                break;
            } else if (value > 255) {
                printf("error: cannot store a value greater than 1 byte at a given address\n");
                readError = 1;
                break;
            } else {
                // no errors, write the byte
                writeFloatToDataCache(cpu->dataCache, address, (float)value);
            }

        // proces instructions
        } else {

            int size = strlen(buf);

            if (size == 0) {
                printf("error: tried to process instruction that was empty\n");
                readError = 1;
                break;
            }

            // strip leading and tailing whitespace from the buffer
            char *start = buf;
            char *end = start + size - 1;
            int numLeadingSpaces = 0;
            int numTailingSpaces = 0;
            
            while (*start && isspace(*start)) {
                start++;
                numLeadingSpaces++;
            }

            while (end >= start && isspace(*end)) {
                end--;
                numTailingSpaces++;
            }
            *(end + 1) = '\0';

            // copy the line to the instruction char buffer
            char *instStr = malloc(bufSize * sizeof(char));

            memcpy(instStr, start, bufSize - numLeadingSpaces - numTailingSpaces);

            // check if the instruction has a label and if so, add it to the label table
            // this is done so that branch instructions are able to get the target address which is normally encoded into the instruction by an assembler / compiler as an offset of the branch's address
            char *colon = strstr(instStr, ":");
            if (colon) {
                int colonIndex = colon - instStr;
                char *label = malloc((colonIndex + 1) * sizeof(char));
                memcpy(label, instStr, colonIndex);
                label[colonIndex] = '\0';

                addEntryToLabelTable(cpu->labelTable, numInsts * 4, label);
            }

            if (!strcmp(instStr, "")) {
                printf("error: tried to process instruction that was empty, not reading any more instructions\n");
                readError = 1;
                break;
            }

            // add the parsed instruction to the cache
            addInstructionToCache(cpu->instCache, instStr);
            numInsts++;
        }      
    }
    fclose(fp);

    if (readError) {
        printf("an error was encountered while processing the input file, exiting...\n");
        exit(1);
    }
}

int main(int argc, char *argv[]) {

    // check number of input files
    if (argc != 3) {
        printf("error: invalid number of inputs provided\nusage: './cpu_sim <config_file> <input_file>'\n");
        return 1;
    }

    // process config file
    Params params;
    readConfig(argv[1], &params);

    // create new CPU struct and initialize it
    CPU cpu;
    initCPU(&cpu, &params);
    processInput(argv[2], &cpu);

    printf("initial ");
    printDataCache(cpu.dataCache);

    // start executing instructions
    executeCPU(&cpu);

    return 0;
}