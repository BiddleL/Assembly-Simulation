// COMP1521 20T3 Assignment 1: mips_sim -- a MIPS simulator
// starting point code v0.7 - 21/10/20


// PUT YOUR HEADER COMMENT HERE


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "decode.h"

#define MAX_LINE_LENGTH 256
#define INSTRUCTIONS_GROW 64


void execute_instructions(int n_instructions,
                          uint32_t instructions[n_instructions],
                          int trace_mode);
char *process_arguments(int argc, char *argv[], int *trace_mode);
uint32_t *read_instructions(char *filename, int *n_instructions_p);
uint32_t *instructions_realloc(uint32_t *instructions, int n_instructions);


// ADD YOUR FUNCTION PROTOTYPES HERE

// YOU SHOULD NOT NEED TO CHANGE MAIN

int main(int argc, char *argv[]) {
    int trace_mode;
    char *filename = process_arguments(argc, argv, &trace_mode);

    int n_instructions;
    uint32_t *instructions = read_instructions(filename, &n_instructions);

    execute_instructions(n_instructions, instructions, trace_mode);

    free(instructions);
    return 0;
}


// simulate execution of  instruction codes in  instructions array
// output from syscall instruction & any error messages are printed
//
// if trace_mode != 0:
//     information is printed about each instruction as it executed
//
// execution stops if it reaches the end of the array

void execute_instructions(int n_instructions,
                          uint32_t instructions[n_instructions],
                          int trace_mode) {

    int pc = 0;
    int32_t reg[32] = {0};
    while (pc < n_instructions && pc >= 0) {
        if (trace_mode) {
            int type = find_instruction(instructions[pc]);
            int set_reg = register_to_set(instructions[pc]);
            printf("%d: 0x%08X ", pc, instructions[pc]);
            if (type != ERROR) {
                printing_instruction_type(instructions[pc]);
                printing_registers(instructions[pc]);
                if 
                (type != SYS && set_reg >= 0 && set_reg < 32) 
                {
                    if(type == BEQ || type == BNE) {
                        print_equals(instructions[pc], reg, pc);
                        pc += operations(instructions[pc], reg);
                    } else {
                        reg[set_reg] = operations(instructions[pc], reg);
                        printf(">>> $%d = %d\n", 
                        set_reg, reg[set_reg]);
                    }
                } else {
                    int check = syscall_print(instructions[pc], reg);
                    if (check == ERROR) pc = n_instructions + 1;
                }
            } else {
                printf("invalid instruction code\n");
                pc = n_instructions +1;
            };
        } else {
            int type = find_instruction(instructions[pc]);
            int set_reg = register_to_set(instructions[pc]);
            if (type != SYS && set_reg >= 0 && set_reg < 32) 
            {
                if(type == BEQ || type == BNE) {
                    pc += operations(instructions[pc], reg);
                } else {
                    reg[set_reg] = operations(instructions[pc], reg);
                }
            } else if (type == SYS) {
                int check = syscall_print_r(instructions[pc], reg);
                if (check == ERROR) pc = n_instructions + 1;
            } else {
                printf("invalid instruction code\n");
                pc = n_instructions +1;
            }

        }
        reg[0] = 0; // register 0 is always 0
        pc++;
    }
    if (pc < 0) printf("Illegal branch to address before instructions: PC = %d\n"
    , pc);
}


// YOU DO NOT NEED TO CHANGE CODE BELOW HERE


// check_arguments is given command-line arguments
// it sets *trace_mode to 0 if -r is specified
//          *trace_mode is set to 1 otherwise
// the filename specified in command-line arguments is returned

char *process_arguments(int argc, char *argv[], int *trace_mode) {
    if (
        argc < 2 ||
        argc > 3 ||
        (argc == 2 && strcmp(argv[1], "-r") == 0) ||
        (argc == 3 && strcmp(argv[1], "-r") != 0)) {
        fprintf(stderr, "Usage: %s [-r] <file>\n", argv[0]);
        exit(1);
    }
    *trace_mode = (argc == 2);
    return argv[argc - 1];
}


// read hexadecimal numbers from filename one per line
// numbers are return in a malloc'ed array
// *n_instructions is set to size of the array

uint32_t *read_instructions(char *filename, int *n_instructions_p) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        fprintf(stderr, "%s: '%s'\n", strerror(errno), filename);
        exit(1);
    }

    uint32_t *instructions = NULL;
    int n_instructions = 0;
    char line[MAX_LINE_LENGTH + 1];
    while (fgets(line, sizeof line, f) != NULL) {

        // grow instructions array in steps of INSTRUCTIONS_GROW elements
        if (n_instructions % INSTRUCTIONS_GROW == 0) {
            instructions = instructions_realloc(instructions, n_instructions + INSTRUCTIONS_GROW);
        }

        char *endptr;
        instructions[n_instructions] = strtol(line, &endptr, 16);
        if (*endptr != '\n' && *endptr != '\r' && *endptr != '\0') {
            fprintf(stderr, "%s:line %d: invalid hexadecimal number: %s",
                    filename, n_instructions + 1, line);
            exit(1);
        }
        n_instructions++;
    }
    fclose(f);
    *n_instructions_p = n_instructions;
    // shrink instructions array to correct size
    instructions = instructions_realloc(instructions, n_instructions);
    return instructions;
}


// instructions_realloc is wrapper for realloc
// it calls realloc to grow/shrink the instructions array
// to the speicfied size
// it exits if realloc fails
// otherwise it returns the new instructions array
uint32_t *instructions_realloc(uint32_t *instructions, int n_instructions) {
    instructions = realloc(instructions, n_instructions * sizeof *instructions);
    if (instructions == NULL) {
        fprintf(stderr, "out of memory");
        exit(1);
    }
    return instructions;
}