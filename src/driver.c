#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "cpu.h"
#include "misc/misc.h"
#include "memory/memory.h"

// populates the params struct with values from a config file
void readConfig(char *configFn, Params *params) {

    printf("processing config file...\n");

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
                // continue; not needed as it's the end of the loop
            }
        }

        fclose(fp);
    }

    printf("parameters:\n\tNF = %d\n\tNI = %d\n\tNW = %d\n\tNR = %d\n\tNB = %d\n", params->NF, params->NI, params->NW, params->NR, params->NB);
}

// process input file
void processInput(char *inputFn, CPU *cpu) {

    printf("processing input program file...\n");
    
    FILE *fp = fopen(inputFn, "r");
    if (fp == NULL) {
        printf("error: could not open input file, exiting...\n");
        exit(1);
    }

    int bufSize = 1024;
    char buf[bufSize];
    int isProcessingInsts = 0;
    int readError = 0;

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
                writeByteToDataCache(cpu->dataCache, address, (unsigned char)value);
            }

        // proces instructions
        } else {

            // initialize instruction
            Instruction inst;
            inst.type = INST_TYPE_NONE;
            inst.destReg = NULL;
            inst.source1Reg = NULL;
            inst.source2Reg = NULL;
            inst.destPhysReg = PHYS_REG_NONE;
            inst.source1PhysReg = PHYS_REG_NONE;
            inst.source2PhysReg = PHYS_REG_NONE;
            inst.imm = 0;
            inst.label[0] = '\0';
            inst.branchTargetLabel[0] = '\0';
            inst.branchTargetAddr = 0;
            inst.regsWereRenamed = 0;
            enum InstructionType type;

            // get the first token of the line
            char *cur = strtok(buf, " \n\t,");

            if (cur == NULL) {
                break;
            }

            // the first token containing a colon indicates it is a label
            if (strstr(cur, ":")) {
                // printf("found label: %s\n", cur);
                
                memcpy(inst.label, cur, strlen(cur)-1);
                inst.label[strlen(cur)-1] = '\0';

                cur = strtok(NULL, " \n\t,");
            }
            
            // convert the instruction type to an enum
            inst.type = stringToInstructionType(cur);

            // instruction type could not be matched
            if (inst.type == INST_TYPE_NONE) {
                
                printf("error: invalid instruction type '%s'\n", cur);
                readError = 1;
                break;

            // memory access instructions
            } else if (inst.type == FLD || inst.type == FSD) {

                inst.destReg = stringToArchRegister(strtok(NULL, " \n\t,"));
                inst.imm = atoi(strtok(NULL, " \n\t,()"));
                inst.source1Reg = stringToArchRegister(strtok(NULL, " \n\t,"));

                if (!inst.destReg || !inst.source1Reg) {
                    printf("error: invalid register in instruction: '%s'\n", buf);
                    readError = 1;
                    break;
                }

            // register-immediate instruction
            } else if (inst.type == ADDI) {

                inst.destReg = stringToArchRegister(strtok(NULL, " \n\t,"));
                inst.source1Reg = stringToArchRegister(strtok(NULL, " \n\t,"));
                inst.imm = atoi(strtok(NULL, " \n\t,"));

                if (!inst.destReg || !inst.source1Reg) {
                    printf("error: invalid register in instruction: '%s'\n", buf);
                    readError = 1;
                    break;
                }

            // register-register instructions
            } else if (inst.type == ADD || inst.type == SLT || inst.type == FADD || inst.type == FSUB || inst.type == FMUL || inst.type == FDIV) {
                
                inst.destReg = stringToArchRegister(strtok(NULL, " \n\t,"));
                inst.source1Reg = stringToArchRegister(strtok(NULL, " \n\t,"));
                inst.source2Reg = stringToArchRegister(strtok(NULL, " \n\t,"));

                if (!inst.destReg || !inst.source1Reg || !inst.source2Reg) {
                    printf("error: invalid register in instruction: '%s'\n", buf);
                    readError = 1;
                    break;
                }

            // branch instruction
            } else if (inst.type == BNE) {

                inst.source1Reg = stringToArchRegister(strtok(NULL, " \n\t,"));
                inst.source2Reg = stringToArchRegister(strtok(NULL, " \n\t,"));
                char *targetLabel = strtok(NULL, " \n\t,");
                memcpy(inst.branchTargetLabel, targetLabel, strlen(targetLabel));
                inst.branchTargetLabel[strlen(targetLabel)] = '\0';

                if (!targetLabel) {
                    printf("error: invalid target label in instruction: '%s'\n", buf);
                    readError = 1;
                    break;
                } else if (!inst.source1Reg || !inst.source2Reg) {
                    printf("error: invalid register in instruction: '%s'\n", buf);
                    readError = 1;
                    break;
                }

            } else {
                printf("error: could not match this point should never be reached...\n");
                readError = 1;
                break;
            }

            // printInstruction(inst);

            // add the parsed instruction to the cache
            addInstructionToCache(cpu->instCache, inst);
        }      
    }
    fclose(fp);

    // after all instructions have been parsed, resolve branch target labels to addresses
    int resolveLabelsError = resolveInstLabels(cpu->instCache);

    if (readError || resolveLabelsError) {
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
    printDataCache(cpu.dataCache);
    executeCPU(&cpu);
   

    return 0;
}